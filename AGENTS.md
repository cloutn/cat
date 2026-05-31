# AGENTS.override.md

## 覆盖说明

这些规则用于覆盖仓库根目录同级的 `AGENTS.md`。如果本文件与仓库里的 agent 规则冲突，以本文件为准。

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

## 个人强制规则

1. 所有新建文档都必须放在 `E:\pw\ue\.doc` 目录下面，除非 @caolei 明确指定其他位置。
2. 所有 Markdown 文档（`*.md`）都必须使用中文文件名，文件名要描述主题，不使用 `plan.md`、`design.md`、`document.md`、`checklist.md` 这类泛用英文名。
3. 修改 `.cpp` 和 `.h` 源文件时，千万不要增加或删除文件末尾的空行；必须保持文件原本的 EOF 空行状态。
4. 不要把个人 skill、个人 agent 配置提交到仓库，除非明确决定团队共享。
5. 代码要在保持可读性的情况下，追求简洁。
6. 不要主动运行 build / 编译验证，除非 @caolei 明确要求；默认只做静态检查、diff 检查或用户指定的轻量验证。

## 说明

`AGENTS.override.md` 是 Codex 的本地覆盖入口文件。普通项目文档仍然必须遵守中文文件名规则；这个文件名为了工具识别保留为固定英文名。
