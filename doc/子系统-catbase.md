# 子系统 — catbase（基础层）

> 上级：[架构总览](./架构总览.md)

`catbase` 是引擎自底向上的「公共类型 / 公共契约」层。它**不依赖任何图形 API**，也不依赖上层的 `cat` / `catvulkan`。所有上层模块都通过这里的接口与类型互通，避免出现循环依赖。

## 1. 模块清单

按职责分组：

| 分组 | 头文件 | 作用 |
|------|--------|------|
| 平台与基础类型 | `cat/def.h` | 全局宏（`safe_delete`、`arg_count`）、`ELEM_TYPE` / `KEY_FRAME_TYPE` 等枚举、`Entity` 别名 |
| 接口契约 | `cat/IRender.h` | 抽象渲染接口（VB/IB/Texture/Shader/draw2），所有图形后端实现它 |
| 接口契约 | `cat/IFileProvider.h` | 资源文件抽象（open/read/mmap/exists/close） |
| 变换与相机 | `cat/transform.h`、`cat/camera.h` | 平移/旋转/缩放、视图/投影矩阵（含 OpenGL/Vulkan z-range 切换） |
| 颜色与纹理 | `cat/color.h`、`cat/textureFile.h` | ARGB 宏、`PIXEL` 枚举、引用计数的纹理元数据 |
| 顶点 | `cat/vertex.h` | `ATTR_LOC_*` 顶点 location 枚举、`vertex_color_uv` 等典型顶点结构 |
| 几何 | `cat/box.h` | AABB |
| 动画关键帧 | `cat/keyFrame.h` | `KeyFrame`：时间 + (rotate/move/scale) union |
| 对象 ID | `cat/objectIDMap.h` | 模板 `ObjectIDMap<T>`，基于 `scl::hash_table` |
| 序列化 | `cat/yaml.h`、`cat/yaml.cpp` | 对 rapidyaml 的轻封装（document / node / 迭代器） |
| 名称常量 | `cat/names.h` | `Names::scale` / `translation` / `rotation` 等字符串常量 |
| 第三方桥接 | `cat/cgltf.c`、`cat/cgltf_util.h/.c` | cgltf 单文件库及其 accessor helpers |
| 哈希 | `cat/xxhash.h/.c`、`cat/xxh3.h` | xxHash 算法（贴在源码中，避免独立链接） |
| 字符串 | `cat/string.h` | 引用 `scl::string`，统一别名 |

## 2. 关键设计

### 2.1 IRender —— 渲染抽象边界

`IRender`（`cat/IRender.h`）是 `cat` 层唯一能看见的图形后端接口，包含：

- **VB/IB**：`createVertexBuffer / writeVertexBuffer / readVertexBuffer / release...`，使用 `void*` 表示设备资源。
- **纹理**：`createTexture(filename, ...)`、`createTexture(width, height, pixel)`、`loadImage` 直接给出原始像素。
- **Shader**：`createShader(vs_code, ps_code)`，文本驱动；返回 `void*`（在 Vulkan 后端是 `svkShaderProgram*`）。
- **绘制**：`draw2(...)` 是引擎使用的唯一绘制入口，参数同时携带 mvp、骨骼矩阵、PushConstant 缓冲；保留扩展位但不暴露 RenderPass。
- **设备信息**：`getDeviceWidth / Height`，用于编辑器布局与 viewport 设置。

> 注：`draw2` 命名是历史遗留；它替代了早期更大的 `draw(...)` 接口（在头文件中仍以注释形式保留作参考）。

### 2.2 Transform —— 惰性矩阵

`Transform` 持 `move / scale / rotate(quat)` 三元组，矩阵 `m_matrix` 为 mutable 缓存，配合 `m_changed` 实现「读取时才重算」。`setByMatrix` 反向分解，供从 glTF 节点直接初始化。

### 2.3 Camera —— 双层 Dirty

`Camera` 分别维护 `m_viewDirty / m_projectionDirty`，再合成 `m_dirty`。任意 setter 都只置对应 dirty 位；`matrix() / viewMatrix() / projectionMatrix()` 调用时按需重算。

- 支持透视 / 正交（`setOrtho`）。
- 通过 `scl::z_range` 区分 NDC 范围（OpenGL 是 -1..1，Vulkan 是 0..1），保证 MVP 复用同一 `Camera` 时仍对得齐。
- 正交模式额外提供 `m_orthoRefPosition`，把焦点 Z 当作参考面而非 nearZ，便于编辑器绕物体观察。

### 2.4 KeyFrame / 动画数据

`KeyFrame` 以 `union { quaternion, vector3, vector3 }` 紧凑承载旋转/平移/缩放三种类型；其类型由所属 `AnimationChannel` 的 `m_type` 决定（详见 [子系统-cat.md](./子系统-cat.md) 动画一节）。`operator<` 直接比较时间，便于按时间排序后做二分插值。

### 2.5 yaml —— 对 rapidyaml 的薄包装

`yaml::document` 持 `c4::yml::Tree`，`yaml::node` 是节点视图；自动桥接 `scl::vector3 / vector2i` 与 ryml 的 `to_chars / from_chars`。读写示例直接写在头文件顶部注释中，便于复制。引擎中所有「场景另存为」走这条路径（见 `Scene::save / Object::save`）。

### 2.6 ObjectIDMap<T>

线程不安全的 `id → T*` 映射，专为 `Object::objectByID` 这类查询设计：
- `alloc_id` 自增分配；
- `add / del` 直接读 `obj->id()`；
- `init(maxCount)` 按 `MAX_CONFLICT=8 × maxCount` 预分配 hash 槽。

### 2.7 cgltf 工具

`cgltf_util.h` 给 cgltf 加了几个引擎内反复用到的辅助函数：

- `cgltf_get_accessor_buffer`：直接拿到 accessor 指向的字节地址（处理稀疏数组、view offset）。
- `cgltf_num_components / cgltf_component_size / cgltf_calc_size`：把 glTF 类型尺寸算法集中起来。
- `cgltf_primitive_has_attr`：避免在 `Primitive::load` 中重复字符串比较。

## 3. 命名与依赖约定

- 所有类型置于 `namespace cat`（个别老接口的 `namespace ui` 是历史遗留，正在收敛）。
- 仅依赖 `scl`（数学 / 容器 / 字符串）与 rapidyaml；不依赖 Vulkan / OpenGL / Windows。
- C 文件（`cgltf.c / cgltf_util.c / xxhash.c`）独立编译，C++ 头通过 `extern "C"` 桥接。
- 头文件保持「无重型 include」：跨模块出现的 `scl::matrix` 等大类型一律前置声明，仅在 `.cpp` 中完整包含。
