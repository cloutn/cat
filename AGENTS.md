# AGENTS.md

## 项目背景

1. cat 是一个面向学习与实验的轻量 3D 引擎，强调"小而清晰、每层职责可替换"。
2. 主要平台是 Windows，渲染后端以 Vulkan 为主（`def.h` 默认开启 `TEST_VULKAN`），保留 OpenGL ES 代码痕迹但不再使用。
3. 资源以 glTF 2.0 为唯一模型/动画交换格式（cgltf 解析）；自带极简编辑器（ImGui + ImGuizmo）含场景树 / 属性面板 / Gizmo / Pick Pass 拾取。
4. 自带 `scl` 替代 STL（容器 / 数学 / IO / 线程 / log），上层模块统一使用 scl，**禁止 STL 容器穿透接口**。
5. 模块分层（自底向上）：`free/scl` + 第三方 → `catbase`（基础类型）→ `cat` + `catvulkan`（引擎层 + Vulkan 后端）→ `catui` + `catwindows`（UI / 平台）→ `testCat`（应用层 / 编辑器）。依赖方向严格自上而下。
6. 主要语言 C++ / HLSL / GLSL；shader 路径 `SHADER_PATH` 由 `TEST_VULKAN` 决定，分别指向 `shader/vulkan/` 或 `shader/opengles/`。
7. 当前主线为**单线程**：游戏循环、渲染提交、资源加载都跑在主线程，cat 模块内不引入 mutex / atomic / thread。如未来引入多线程（RHI 线程 / async load），需要先在本节补完整的线程模型说明再动代码。

## 接口与依赖约束

1. 上层只通过 `cat::IRender` 与图形 API 交互，**`cat` 模块禁止直接 include `<vulkan/...>` / `<shaderc/...>` / `<spirv_cross/...>`**；Vulkan 句柄不暴露给 `cat`，`IRender` 中以 `void*` 表示设备资源句柄。
2. `catvulkan` 是 `IRender` 的具体实现层，可见 Vulkan 全部头；`catbase` 不依赖 `cat` / `catvulkan` / `catui` / `catwindows`；`catwindows` 是平台层（Win32 / EGL）；`catui` 仅依赖 ImGui + scl。
3. 命名空间：引擎代码 `namespace cat`；基础库 `namespace scl`；编辑器配置 `namespace game`；UI 扩展 `namespace imguiex`；YAML 包装 `namespace yaml`。

## 代码风格

1. 权威来源是 `.cursor/rules/cat代码风格.mdc`（`alwaysApply: true`，自动加载）。**所有具体风格规则以该文件为准**，本节只做高频提醒，不复述细节。
2. 关键硬规则速记：
   - 用 `NULL`，不用 `nullptr`；空指针比较用 Yoda 风格 `if (NULL == ptr)`。
   - 删除指针用宏 `safe_delete` / `safe_delete_array`，禁止裸 `delete ptr; ptr = NULL;`。
   - 禁止 STL 容器 / `std::string` 出现在头文件接口或类成员；用 `scl::varray` / `scl::array` / `scl::tree` / `scl::hash_table` / `cat::String`。
   - 不抛异常；用返回值（`bool` / `int` / NULL 指针）+ `assert()` 报错。
   - 类成员声明、init list、头文件方法表保留**纵向对齐**风格。
   - Tab 缩进，Allman 大括号风格。
   - 用 `#pragma once`，不用 include guard 宏。
3. 命名取舍：**简洁服从一致**。优先沿用仓库已有命名模式（如"按属性查找"统一走 `xxByY` 形式：`objectByID` / `childByID` / `objectByName` / `childByName`），不引入与已有模式同义但措辞冗余的新前缀（如 `findXxxByY` 与 `xxByY` 同义重复）。最短形式只在不与现有重载冲突、不引入歧义的前提下选择（参考 `child(int)` 与 `childByName(const char*)` 拆分的处理）。
4. 详细规则（命名前缀、include 顺序、平台宏、12 节自检清单等）直接读 `cat代码风格.mdc`，不在此重述。

## 文件编码与 Windows 终端注意事项

1. 所有源文件（`.cpp` / `.h` / `.hlsl` / `.glsl` / `.usf` / `.ush`）和文档（`.md`）**统一使用 UTF-8 无 BOM**，与仓库现状保持一致（截至定稿，`cat/` 下 42 个 cpp/h 全部 UTF-8 无 BOM，含中文注释的文件也无 BOM）。修改时**禁止给文件添加 BOM、也禁止移除已有 BOM**，避免单个文件破坏仓库的编码一致性。
2. MSVC 在中文 Windows 上对无 BOM 的 UTF-8 源码会按 GBK 解码并产生 C4819 等问题，解决方案是**给编译选项加 `/utf-8`**（同时设置 source charset 和 execution charset），**不靠 BOM 解决**。
3. 修改文件时必须保留文件原有的编码与换行符（CRLF/LF），不要顺手"统一"换行风格，diff 会变成全文件改动。
4. **严禁使用 PowerShell / cmd 的重定向写入含非 ASCII 字符的文件**：
   - 不要用 `echo "中文" > a.md`、`"内容" | Out-File a.md`、`Set-Content a.md "中文"` 这类命令产出含中文的文件；Windows PowerShell 5.x 默认以系统代码页（中文 Windows 是 GBK / CP936）写盘，会把内容写成 GBK，并且常常带上 UTF-16 BOM，造成跨平台/编译/Git diff 全乱。
   - 写文件**只用编辑器工具**（`Write` / `StrReplace` / `EditNotebook`），不要走 shell 重定向。
5. **读文件不要走 shell**：不要用 `type`、`cat`、`Get-Content` 把中文源文件打印到终端再"看"内容，PowerShell 控制台默认 OEM 代码页（CP936/CP437）会把 UTF-8 字节当成 GBK 解码，看到的全是乱码，容易做出错误判断。读文件统一用 `Read` 工具。
6. 必须在终端里跑命令时（如 git、构建脚本）：
   - 优先一次性切换会话编码：`chcp 65001` + `[Console]::OutputEncoding = [System.Text.Encoding]::UTF8`；
   - 给 PowerShell cmdlet 显式加 `-Encoding UTF8`（如 `Get-Content -Encoding UTF8`），不要依赖默认值；
   - 调 `git` 时如果路径或提交信息含中文，确认 `core.quotepath=false`、`i18n.commitEncoding=utf-8`、`i18n.logOutputEncoding=utf-8` 已配置。
7. 终端命令里**避免出现中文字面量参数**（中文路径、中文 commit message、中文 grep pattern）。需要这些操作时，改为：先把内容写进文件，再让命令读文件；或者使用专门的编辑/搜索工具（`Grep` / `Glob` / `Read` / `StrReplace`）而不是 shell。
8. 任何一次会话里如果观察到终端输出疑似乱码（出现 `��`、`�����` 之类），立即停止把该输出当作可信信息源，改用文件工具复核，不要基于乱码做后续判断或修改。

## 个人强制规则

1. 所有新建文档都必须放在 `E:\pw\ue\.doc` 目录下面，除非 @caolei 明确指定其他位置。
2. 所有 Markdown 文档（`*.md`）都必须使用中文文件名，文件名要描述主题，不使用 `plan.md`、`design.md`、`document.md`、`checklist.md` 这类泛用英文名。
3. 修改 `.cpp` 和 `.h` 源文件时，千万不要增加或删除文件末尾的空行；必须保持文件原本的 EOF 空行状态。
4. 不要把个人 skill、个人 agent 配置提交到仓库，除非明确决定团队共享。
5. 代码要在保持可读性的情况下，追求简洁。
6. 不要主动运行 build / 编译验证，除非 @caolei 明确要求；默认只做静态检查、diff 检查或用户指定的轻量验证。

## 说明

`AGENTS.md` 是仓库公共的 agent 入口文件，给 Cursor / Codex / 其他兼容 `AGENTS.md` 协议的工具自动加载。如果存在同目录下的 `AGENTS.override.md`，以 override 文件为准。`AGENTS.md` 与 `AGENTS.override.md` 这两个文件名为了工具识别保留固定英文名，不受"中文文件名"规则约束。
