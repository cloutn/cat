# AGENTS.md

## 项目背景

1. 这是 UE4.26 深度定制引擎。
2. 主要平台是 Android，其次是 iOS 和 PC。
3. 渲染管线是 Forward Rendering。
4. 图形 API 主要涉及 OpenGL ES 3.1 和 DirectX 11。
5. 目标设备跨度从 Snapdragon 660 到 Snapdragon 8 Gen 3。
6. 主要语言是 C++ / HLSL / GLSL。
7. Shader 目录是 `Engine/Shaders/`。

## 线程模型

1. 主要线程链路是 GameThread -> RenderThread -> RHIThread。
2. RHIThread 已启用，RenderThread 会异步提交 RHI 命令。
3. 修改渲染代码时，特别注意 RenderThread/RHIThread 访问 GameThread 数据导致的悬垂指针、生命周期和同步问题。

## 性能目标

1. 目标分辨率：1080p。
2. 目标帧率：60 FPS。
3. 典型 DrawCall：500+，其中 foliage 250+。
4. 典型三角形：约 800K。
5. 移动端性能优先关注内存分配、API flush、RenderTarget load/store、带宽和 DrawCall。

## 代码风格

1. 遵循 UE idiom，不写脱离 UE 架构的泛 C++。
2. 优先使用 UE 容器、UE 内存管理、UE delegate、UE 线程/RHI 模式。
3. 日志使用当前模块或当前功能对应的 log category，不随便用泛用 category。
4. invariant 检查优先考虑 `check()` / `ensure()`，但线上容错路径要能 fail closed。

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
