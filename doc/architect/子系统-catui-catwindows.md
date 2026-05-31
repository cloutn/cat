# 子系统 — catui / catwindows（UI 与窗口/平台层）

> 上级：[架构总览](./架构总览.md)

这两个模块都很薄，合并到一份文档中：

- `catui`：业务层 ImGui 控件扩展 + 第三方 ImGuizmo 源码。
- `catwindows`：Win32 窗口、Win32 ImGui 平台 backend、可选的 EGL/OpenGL ES 窗口。

它们只在 Windows 平台被启用（其他平台需要补对应实现），但对外接口都做了平台无关化处理。

## 1. catui

### 1.1 模块组成

| 文件 | 作用 |
|------|------|
| `cat/imguiex.h/.cpp` | 业务 ImGui 控件扩展。`namespace imguiex` |
| `cat/ImGuizmo.h/.cpp` | 第三方 [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) 源码（v1.89 WIP），用作 3D 操纵手柄 |

### 1.2 imguiex 扩展控件

`imguiex.h` 把一组反复使用的"标签 + 数值控件"封装成一行调用，签名都按 `(const char* label, T& value)` 设计：

- 标签 + 文本：`labelText(name, fmt, ...)`、`inputText(label, char* text, capacity)`
- 数值：`inputFloat / inputFloat2/3/4`、`inputDouble`、`inputInt2`、`inputHex`
- 矩阵：`inputMatrix4(label, scl::matrix&)`（4×4 编辑）
- 颜色：`inputColorFloat(label, scl::vector4&)`、`inputColorInt(label, uint32&)`
- 布尔：`checkbox(label, bool&)`
- 左对齐标签助手：`leftLable(name)` 返回 `scl::string256`，配合宏 `LEFT_LABEL("Name")` 直接喂给 `ImGui::*` 控件，统一两栏布局风格。

> 这些控件直接读写 `scl::vector*` / `scl::matrix`，避免业务代码在 ImGui 原生 float 数组与引擎类型之间反复转换。

### 1.3 ImGuizmo

未做修改的第三方组件，提供：

- `ImGuizmo::Manipulate(view, projection, OPERATION, MODE, matrix, ...)` —— 屏幕空间 Translate / Rotate / Scale 手柄；
- 左上角 ViewGizmo（`ViewManipulate`）—— 类似 Unity 的轴向指示器（任务 `task.txt` 中已规划改进项）。

editor 端 `MainGUI::_processGizmo / _operateLocal / _operateGlobal*` 通过它驱动 `Object` 的 transform 写回。

## 2. catwindows

### 2.1 模块组成

| 文件 | 作用 |
|------|------|
| `cat/win32window.h/.cpp` | `Win32Window`：窗口创建、消息循环、事件分发 |
| `cat/imgui_impl_win32.h/.cpp` | ImGui 官方 Win32 后端（无定制） |
| `cat/eglWindow.h/.cpp` | 可选的 EGL/OpenGL ES 窗口（`EGLWindow`），用于以 GLES 驱动 OpenGL 后端路径 |

### 2.2 Win32Window

`Win32Window`（`win32window.h`）是窗口与事件抽象：

- `init(x, y, w, h, title, icon, enableDpiAwareness)`：注册类、创建 HWND，可选 DPI awareness。
- `run()`：跑一次消息泵（PeekMessage + Translate/Dispatch），由上层主循环按帧调用。
- **事件分发**：内置最多 `MAX_EVENT_HANDLER = 128` 个 `EventHandlerFuncT`，`registerEventHandler / unregister` 用于挂载，`WndProc` 在收到 Win32 消息时按注册顺序广播；任一 handler 返回 `true` 即视为消费。
- `postEvent(hWnd, message, wParam, lParam)`：手工广播给所有 handler（不经 Win32 队列），编辑器某些合成事件用得到。
- 句柄访问：`getHandle()` 返回 `HWND`，`getInstance()` 返回 `HINSTANCE`，交给 `VulkanRender::init(hInstance, hwnd)` 建 surface。

### 2.3 imgui_impl_win32

直接采用官方 backend（无显著改动）：

- `ImGui_ImplWin32_Init(hwnd)` / `Shutdown / NewFrame`；
- 在 `Win32Window` 的 `EventHandlerFuncT` 中调用 `ImGui_ImplWin32_WndProcHandler` 转发输入；
- 渲染部分由 `VulkanRender::initIMGUI / drawIMGUI` 负责（即 `imgui_impl_vulkan` 配套）。

### 2.4 EGLWindow（可选 GLES 通路）

`EGLWindow`（`eglWindow.h`）封装 EGL 显示/上下文/Surface 的创建与切换：

- `create(hwnd)`：把 HWND 绑到 EGL surface；
- `swap()`：`eglSwapBuffers`；
- `update_size`：surface 大小变化时同步。

这是 OpenGL ES 后端（已弃用主线，但代码保留）唯一需要的窗口胶水层。当前主线使用 Vulkan，`EGLWindow` 不被默认链接到运行时路径。

## 3. 与上层的契约

- `testCat` 持有 `Win32Window m_window`；窗口事件通过 `Win32Window::registerEventHandler` 注册 `Client::onEvent`、ImGui WndProc、ImGuizmo 等。
- `Win32Window` 只暴露 `void*` HWND / HINSTANCE，不在公共头中包含 `<windows.h>`，避免污染上层编译单元。
- imgui 渲染分两段：平台 backend（这里的 `imgui_impl_win32`）+ 渲染 backend（`catvulkan/imgui_impl_vulkan`），两者解耦，可换平台或换渲染 API。
