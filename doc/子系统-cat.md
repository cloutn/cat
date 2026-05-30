# 子系统 — cat（引擎层）

> 上级：[架构总览](./架构总览.md)

`cat` 模块是引擎核心：场景图、资源管理、材质 / Shader、动画、特殊渲染对象（地形 / 天空盒 / 灯光）。它**只依赖** `catbase` + `scl` + `rapidyaml`，通过 `IRender*` 间接驱动具体的 GPU 后端。

## 1. 模块组成

| 子模块 | 主要类型 | 职责 |
|--------|----------|------|
| 场景图 | `Scene`、`Object` | 顶层场景容器 + 节点树（含 transform、子节点、组件、Skin、Mesh） |
| 网格 | `Mesh`、`Primitive` | Mesh 是 Primitive 数组；Primitive 持有 VB/IB/Shader/Material |
| 材质 | `Material` | 持纹理引用 + IRender；从 cgltf_material 加载 |
| Shader | `Shader`、`ShaderCache`、`ShaderMacro(Array)` | 文本 shader + 宏变体 + tree 缓存 |
| 资源 / 全局 | `Env` | 引擎级"上下文"：ShaderCache、纹理表、Pick 表、gltf 节点映射 |
| 骨骼动画 | `Skin`、`Animation`、`AnimationChannel`、`KeyFrame`（catbase） | 关节绑定 + 时间通道 + 关键帧插值 |
| 特殊渲染 | `Terrain`、`Skybox`、`Light`、`Grid` | 地形小网格、天空颜色、点光源占位、辅助网格 |
| ECS 实验 | `World`、`Component`、`ComponentArray` | 极简 ECS 雏形：`addComponent<T>` 动态分桶 |
| 加载/调试 | `GltfLoader`、`gltf_raw_render` | glTF 加载入口 + 不进入引擎对象树的"裸渲染"路径 |
| 平台 Shader | `shader_gles.h/.cpp` | 一组内嵌的 GLES2/3 shader 源（参考实现，主线不再使用） |

## 2. 场景图

### 2.1 Scene

`Scene`（`cat/scene.h`）是单个 glTF scene 的容器：

- `load(cgltf_scene&, path, Env*)`：按 gltf_scene::nodes 创建顶级 `Object`，递归 `loadNode`，最后再走一遍 `loadSkin`（Skin 需要等所有节点创建完成才能解析关节引用）。
- `draw(mvp, isPick, IRender*)`：遍历顶级 `Object::draw`。
- `findObject` / `objectByID`：递归搜索；ID 走 `Object::objectByID`，全局唯一。
- `save(filename)`：通过 `yaml::document` 序列化场景树（每个 `Object::save` 写自身 transform / 子节点）。

### 2.2 Object

`Object`（`cat/object.h`）是场景树节点，主要属性：

- 唯一 `m_id`（来自全局静态 `ObjectIDMap<Object>`）；
- `m_parent / m_childs` 构成节点树；
- `m_transform`（`Transform*`）—— **延迟分配**，没有空间变化的纯组节点可以不创建；
- `m_mesh` / `m_skin` 可选；
- `m_fixComponents` 固定槽位 + `m_components` 可变数组的双轨组件结构（`FIX_COMPONENT_TYPE_TRANSFORM` 等热点组件走固定槽，其他走数组）。

矩阵相关 API：

- `matrix()` 局部变换；
- `globalMatrix()` 沿父链相乘；
- `parentGlobalMatrix()` / `parentGlobalMatrixInverse()`：编辑器中 Gizmo 用来在"父空间 / 世界空间"之间换算。

### 2.3 World / Component（ECS 雏形）

`World`（`cat/world.h`）独立于 `Scene` / `Object`，是一个**实验性 ECS**：

- `createEntity()` 自增 ID；
- `addComponent<T>(e, c)` 用 `scl::type_id<T>()` 作 key，缺则创建 `ComponentArray<T>`；
- 目前主程序里只有一个调用（`main.cpp` 加 `TestComponent`），未与渲染主线耦合，留作演进。

## 3. 渲染对象

### 3.1 Mesh

`Mesh`（`cat/mesh.h`）= Primitive 列表。`load(cgltf_mesh*, path, skinJointCount, parent, IRender*, Env*)` 按 glTF mesh.primitives 逐个加载 `Primitive`。

- `setEnableSkin` 会沿 Primitive 数组下发，影响后续 `draw` 是否走 skinned shader。
- `boundingBox()` 聚合所有 Primitive 的 box。

### 3.2 Primitive

`Primitive`（`cat/primitive.h`）是真正的 draw 单元：

- **顶点数据**：`m_deviceVertexBuffers` 多 buffer + `m_attrs` / `m_attrBufferIndices`，与 glTF 的「多 bufferView 一 primitive」对齐。
- **索引数据**：`m_deviceIndexBuffer` + `m_indexComponentType`（uint16/uint32）。
- **Shader / Material**：自带 `m_shader` 和 `m_pickShader`（拾取时切换），由 `setShaderWithPick` 一次性设置。
- **拾取**：通过 `Env::registerPickPrimitive(primitive)` 拿到唯一颜色，在 draw 时写入 push constant；详见 [架构总览 §4.3](./架构总览.md#43-拾取pick-pass)。
- **顶点查询**：`vertexPosition / vertexPositions / vertexAttr(s)` 从 GPU buffer 读回（`IRender::readVertexBuffer`），编辑器侧的 Box / Gizmo / 拾取辅助用得到。

### 3.3 Material

`Material`（`cat/material.h`）目前是「单纹理」材质：仅持 `TextureFile*`。`load(cgltf_material*, ...)` 解析 baseColorTexture；`init(render, filename, env)` 用于程序化构造。后续 PBR 扩展点：在这里增加 normal / metallicRoughness 等纹理槽。

### 3.4 Shader 与 ShaderCache

- `Shader`（`cat/shader.h`）：持 vs/fs 文件名 + 宏数组 + `m_deviceShader`。`shader(IRender*)` 是惰性编译入口：dirty 时重新读文件、拼宏，调用 `IRender::createShader(vs_code, ps_code)` 编译。
- `ShaderMacro / ShaderMacroArray`（`cat/shaderMacro.h`）：键值对集合，支持去重添加 / 删除；用于派生 shader 变体。
- `ShaderCache`（`cat/shaderCache.h`）：`scl::tree<String, Shader*>`，键由"vs + fs + 排序后的宏列表"组成。提供 `addMacro / removeMacro` 这种「在已有 shader 基础上派生」的便捷方法，以及 `getPickShader(shader)`（在原 shader 上加 `PICK` 宏的变体）。
- 文件路径：`SHADER_PATH` 在 `def.h` 中由 `TEST_VULKAN` 决定，分别对应 `shader/vulkan/` 与 `shader/opengles/`。

### 3.5 Env —— 引擎上下文

`Env`（`cat/env.h`）是「单帧场景渲染所需的全局状态」聚合点：

- `m_render`：`IRender*`，向下传递；
- `m_shaderCache`：唯一的 shader 池；
- `m_textureFiles`：`scl::tree<string256, TextureFile>`，按文件名引用计数共享纹理；
- `m_gltfNodeMap`：`cgltf_node* → ObjectID`，让 Skin 的 joints 能反查到已经创建的 `Object`；
- `m_pickPrimitives`：拾取表，最大 1024 项；`registerPickPrimitive` 返回一个唯一编码颜色（vec4）。

## 4. 动画

### 4.1 Skin

`Skin`（`cat/skin.h`）持有：

- `m_inverseBindMatrices`：来自 glTF 的逆绑定矩阵数组；
- `m_joints`：关节 `Object*` 列表（通过 `Env::getObjectByGltfNode` 反查得到）；
- `generateJointMatrix(inverseMeshGlobalTransform)`：每帧从关节 `globalMatrix()` × `inverseBind` × `inverseMeshGlobal` 算出最终 joint matrix，交给 `Primitive::draw(... jointMatrices, ...)`。

### 4.2 Animation / AnimationChannel

- `Animation`（`cat/animation.h`）：通道集合，持 `m_time`；`update(diff)` 把所有 channel 推进一步并 apply。
- `AnimationChannel`（`cat/animationChannel.h`）：单一通道，对应「某个 target 的某种属性」：
  - `m_target`：受影响的 `Object` ID；
  - `m_type`：`KEY_FRAME_TYPE`（rotate / move / scale）；
  - `m_frames`：按时间排序的 `KeyFrame*`；
  - `update(time)` 找前后两帧 → `_lerp` 算插值 → 缓存到 union；
  - `apply()` 把缓存写回 `Object::setRotate/Move/Scale`。
- `KeyFrame` 在 `catbase` 中定义，用 union 紧凑承载三类型（同一帧只可能是其中一种）。

## 5. 特殊渲染对象

| 类型 | 说明 |
|------|------|
| `Terrain` | 固定 4×4 顶点的占位地形，使用 `vertex_color_uv`；走标准 `Primitive::draw` 路径，便于后续扩展（高度图 / RVT 见 `doc/task.txt`）。 |
| `Skybox` | 目前仅一个颜色 + 强度，不绑 mesh；上层把它当作清屏色或环境光源即可。 |
| `Light` | 占位类型：颜色 + Transform；尚未驱动任何 shader uniform，是 PBR 工作前置接口。 |
| `Grid` | 编辑器辅助网格，构造/析构 + 空 `draw()` 占位；具体网格构造在 `client.cpp` 中拼装。 |

## 6. glTF 加载路径对比

| 路径 | 入口 | 是否进入场景图 | 用途 |
|------|------|----------------|------|
| 引擎对象 | `Scene::load → Object::loadNode → Mesh::load → Primitive::load` | 是 | 主要路径：得到可编辑、可拾取、可保存的 `Object` 树 |
| Raw 渲染 | `gltf_raw_render.h`：`gltf_load_from_file → gltf_do_render → gltf_release` | 否 | 调试 / 对照：直接基于 `cgltf_data` 渲染，绕过引擎对象，用于验证 IRender 实现 |

两条路径的 shader 与 IRender 共享，但拾取、动画、Skin 只在引擎对象路径上生效。

## 7. 单帧主循环（在 cat 视角）

```
Scene::draw(mvp, isPick, render)
 └─ for object in topLevel:
      Object::draw(mvp, isPick, render)
        ├─ // 累乘父矩阵 → modelMatrix
        ├─ if hasSkin: skin->generateJointMatrix(...)
        ├─ if mesh:   mesh->draw(mvpModel, jointMatrices, jointCount, isPick, render)
        │     └─ for primitive in mesh.primitives:
        │           primitive->draw(...)
        │             ├─ shader = (isPick ? m_pickShader : m_shader)->shader(render)
        │             ├─ texture = material ? material->texture() : null
        │             └─ render->draw2(texture, vbs, type, ib, ic, ict, ioff,
        │                              attrCount, attrs, shader, mvpModel,
        │                              jointMatrices, jointMatrixCount,
        │                              pushConst, pushConstSize)
        └─ for child in m_childs: child->draw(...)
```

PushConstant 中的颜色 ID 即拾取颜色（仅 Pick Pass 写入有效值），与 [架构总览](./架构总览.md) 中的拾取流程对齐。
