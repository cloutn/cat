# 子系统 — testCat（编辑器 / 示例应用）

> 上级：[架构总览](./架构总览.md)

`testCat` 是引擎的**主可执行程序**。它扮演双重角色：

1. 引擎的入口示例（main 函数、主循环、窗口、相机）；
2. 简易场景编辑器（ImGui + ImGuizmo 提供场景树、属性面板、Gizmo 操作）。

CMake 上 `testCat/CMakeLists.txt` 把所有内部库与第三方依赖串起来，是项目的根构建目标。

## 1. 模块组成

| 文件 | 角色 |
|------|------|
| `main.cpp` | 入口：CRT 内存检查、创建 `World`、构造并运行 `Client`、退出前 `scl::log::release` |
| `client.h/.cpp` | `Client`：主循环、窗口持有、相机、Scene/Animation 集合、拾取、Gizmo 状态、输入回调 |
| `mainGUI.h/.cpp` | `MainGUI`：场景树 / 属性面板 / 工具条 / 菜单 / 弹窗，以及 Gizmo 局部/全局操作 |
| `config.h/.cpp` | `game::Config`：窗口位置/大小、清屏色、各种 demo 窗口开关、是否 reverse-Z；支持 `load/save`（YAML） |
| `simpleShape.h/.cpp` | `_createGrid` / `_createCube` 等程序化 `Object` 工厂 |
| `testPrimitive.h/.cpp` | 单元/集成测试型 Primitive：骨骼可视化、顶点访问 API 边界情况 |
| `shader/opengles/*` `shader/vulkan/*` | 内置 shader 资源（运行期由 `SHADER_PATH` 选择目录） |

## 2. Client —— 应用主对象

### 2.1 生命周期

`Client::init` 做的事情：

1. `m_window.init(...)`：建 Win32 窗口（从 `Config` 读位置/大小）。
2. `m_render.init(hInstance, hwnd)`：建 Vulkan 后端，注册 `OnSurfaceResize`。
3. `m_env = new Env`；设置默认 shader / 默认材质纹理。
4. `m_camera = new Camera`：透视投影，根据 `Config::reverseZ` 决定 `z_range`。
5. `m_gui.init(this)`：初始化 ImGui + ImGui Win32/Vulkan 后端，注册场景树等事件回调。
6. 加载默认 glTF 场景（`loadGltf`），构造 `Grid`、`Terrain`、骨骼线框等编辑器辅助对象。

`Client::run` 是主循环（Windows 路径）：

```
while (m_window.run())
{
    _processKeydown();          // 相机控制 / 快捷键
    updateAnimation(frameDt);   // 推进 m_animations
    m_render.beginDraw();
    if (pick请求) _renderScene(isPick=true)  // Pick Pass
    _renderScene(isPick=false);  // Scene Pass
    m_gui.onGUI(); m_gui.Render(); m_render.drawIMGUI(...)
    m_render.swap();
}
```

`_renderScene(isPick)` 内部按拓扑遍历 `m_scenes`，调用 `Scene::draw(camera->matrix(), isPick, &m_render)`。

### 2.2 输入与拾取

- 鼠标左键拖拽 = 相机轨道旋转（`m_dragging` + `m_dragPrev`）。
- 鼠标右键拖拽 = 相机平移（`m_rightDragging` + `m_rightDragPrev`）。
- 滚轮 = 推拉相机（`Camera::move_front`）。
- 点击 = `_clickSelectObject(x, y)` 触发一次 Pick Pass，反查 `Primitive`，沿 `parentObject()` 链找到对应 `Object`，调用 `setSelectObject`。
- 按键由 `_processKeydown` 翻译为 `OPERATE_TYPE`（Q/W/E/R → 拾取 / 平移 / 旋转 / 缩放），写入 `m_operateType`，下一帧的 Gizmo 使用。

### 2.3 协调对象

`Client` 持有的"大件"对象：

- `m_window`、`m_render`、`m_env`、`m_camera`、`m_gui`、`m_config`；
- `m_scenes`：`scl::varray<Scene*>`，支持多场景同屏；
- `m_animations`：`scl::varray<Animation*>`，每帧统一 `update`；
- `m_terrain`、`m_grid`、`m_bonePrimitive`：编辑器辅助；
- `m_selectObject`、`m_operateType`、`m_transformType`：编辑器选中态与 Gizmo 模式。

## 3. MainGUI —— 编辑器界面

`MainGUI`（`mainGUI.h`）只关心 UI 与编辑命令，不直接修改渲染状态。

### 3.1 UI 结构

由 `onGUI()` 调度，每个面板拆成私有方法：

| 方法 | 内容 |
|------|------|
| `_showMenu` | 顶部菜单栏（场景加载 / 保存、视图开关、退出） |
| `_showToolbar` | 工具栏：操作模式（T/R/S）、Local/World、Pick Pass 按钮等 |
| `_showWindowScene` | 场景树（递归 `_onGUIScene → _onGUIObject`） |
| `_showWindowProperty` | 选中对象的 transform / mesh / skin / 自定义组件编辑 |
| `_showWindowDebug` | FPS、相机信息、内部缓存命中等 |
| `_showWindowDeviceInfo` | `VulkanRender::getDeviceInfo()` 的可视化（详见 `deviceInfo.h`） |
| `_showWindowConfig` | `Config` 字段的实时编辑，支持立即生效 |

### 3.2 Gizmo 操作流

`_processGizmo()` 是每帧 ImGui 阶段最后调用的核心：

1. 根据 `Client::operateType / transformType` 决定 `ImGuizmo::OPERATION + MODE`；
2. 调 `ImGuizmo::Manipulate(view, proj, op, mode, &model[0][0])`；
3. 若用户拖动，得到新的 `model` 矩阵 → 调 `_operateLocal` 或 `_operateGlobal*` 写回到选中 `Object`：
   - **Local 路径**：直接 `Object::setByMatrix(delta * matrix())`；
   - **Global 路径**：需要 `parentGlobalMatrix / Inverse` 把世界空间增量换回父空间，再分别处理平移与旋转（`_operateGlobalTranslate / _operateGlobalRotate / _operateGlobalRotate2` 提供两种旋转策略以应对父链含非均匀缩放的边界）。

### 3.3 事件回调

`MainGUI` 提供小型事件总线：`registerEvent(GUI_EVENT_PICK_PASS_CLICK, lambda)`，被 `Client::OnButtonClick_PickPass` 等业务方挂载。事件枚举集中在 `GUI_EVENT` 中，扩展时增加新枚举值即可。

## 4. 辅助工厂与测试

### 4.1 simpleShape

`simpleShape.h` 提供程序化 `Object`：

- `_createGrid(render, env)`：构造编辑器地面网格（XZ 平面，颜色线段）。
- `_createCube(render, env)`：构造一个带 UV 的彩色立方体，用于"无 glTF 也能验证渲染管线"。

### 4.2 testPrimitive

`testPrimitive.h` 集中了对 `Primitive` 边界 API 的覆盖测试，运行时通过编辑器某个按钮触发：

- `createTestVulkanPrimitive(...)` / `createTestVulkanPrimitiveColor(...)`：直接喂顶点数据构造 Primitive，绕过 cgltf。
- `createBone(root, ...)` + `collectBoneVertices(root, vertices, indices)`：把 skin 关节链构造成线段渲染数据，便于"骨骼可视化"。
- `testPrimitive_vertexAttr / vertexPosition / vertexPosition_edgeCase`：验证 `Primitive::vertexAttr(s) / vertexPosition(s)` 在空数据、单点、不对齐属性等边界下的行为。

## 5. 配置（game::Config）

`Config`（`config.h`）描述一组可持久化的编辑器/运行时配置：

- `screenSize / screenPos`：窗口属性；
- `clearColor`（`uint32 ARGB`）+ `getClearColorf()` 返回 `scl::vector4`；
- `reverseZ`：是否使用反向 Z（Vulkan 0..1 + far→near 提升精度），由 `Camera::setZRange` 配合；
- `showDemoWindow / showDeviceInfoWindow / showConfigWindow`：UI 开关；
- `load(filename) / save(filename)`：基于 `yaml::document` 序列化。

退出前 `Client` 把当前 `m_config` 持久化，下次启动恢复。

## 6. shader 资源

测试程序携带的 shader 位于：

- `testCat/shader/vulkan/*.vert/.frag`：Vulkan 主线 shader（runtime 通过 `shaderc` 编 SPIR-V）。
- `testCat/shader/opengles/*.vert/.frag`：保留的 GLES 路径 shader。

二选一由 `def.h` 的 `TEST_VULKAN` 决定（即 `SHADER_PATH`），上层只用文件名，无需关心后缀目录。
