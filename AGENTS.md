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

以下内容从 `.cursor/rules/cat代码风格.mdc` 同步，供 Codex 自动加载使用。Cursor 仍然读取原 `.mdc` 文件；两处内容如需长期保持一致，请以 `.cursor/rules/cat代码风格.mdc` 为源头重新同步。

# cat 引擎代码风格 — 强制规则

适用范围：本仓库（`cat` / `catbase` / `catui` / `catwindows` / `catvulkan` / `testCat`）所有 `.h` / `.cpp` / `.c` 文件。规则从现有代码归纳而来，**违反任何一条都属于风格 bug**。修改、新增、重构都必须遵守。

> 与 `AGENTS.override.md` / `CLAUDE.override.md` 作用域不重叠：
>
> - **本文件**描述 cat 引擎的代码风格（命名、对齐、`scl::*` 容器、`NULL` vs `nullptr`、init list 列对齐、`#pragma once`、Tab 缩进等），属"具体技术风格"层。
> - **override 文件**描述跨项目的个人元规则与流程约束（文档放置位置、文件命名规则、是否运行编译验证、个人/团队配置入库与否等），属"流程/元规则"层。
>
> 优先级判定原则：
>
> 1. 两份文件的条款**作用域不同**时（例如本文件管"`m_` 前缀"、override 管"新建 markdown 放 `..\pw\ue\.doc`"），各自在自己的范围内生效，互不覆盖。
> 2. 两份文件**对同一条款给出不同要求**时（典型完全重叠条款：EOF 空行），以 override 为准——这与 `personal-override.mdc` 的"最高优先级"声明一致。
> 3. 当 override 对某个**具体技术细节**只给了泛化措辞而本文件给了精确写法时（例如 override 说"代码要简洁"、本文件说"删除指针用 `safe_delete`"），按本文件的具体写法落地，不视为冲突。

## 1. 空指针与内存

1. 用 `NULL`，**不要用 `nullptr`**。所有对比、初始化、参数默认值都用 `NULL`。
2. 删除指针用宏（定义在 `catbase/cat/def.h`）：
   - 单对象：`safe_delete(ptr);`
   - 数组：`safe_delete_array(ptr);`
   - **禁止**直接写 `delete ptr; ptr = NULL;`。
3. 空指针比较用 Yoda 风格：`if (NULL != ptr)`、`if (NULL == m_mesh)`，与现有代码（`object.cpp`、`primitive.cpp` 等）保持一致。
4. 不使用智能指针（`std::unique_ptr` / `std::shared_ptr`）。如确实需要引用计数语义，用 `scl::ptr`。
5. 不抛异常；用返回值（`bool` / `int` / `NULL` 指针）报错；用 `assert()` / `scl::assert.h` 检查不变量。

## 2. 容器与基础类型

6. **禁止** `std::vector / std::array / std::map / std::unordered_map / std::set / std::string` 出现在头文件接口或类成员中。`.cpp` 内部局部使用（如 `scene.cpp` 的 `std::queue`）属于既有破例，不要扩散。
7. 用 `scl::varray`（动态数组）、`scl::array<T,N>`（固定容量栈数组）、`scl::tree`（按 key 平衡树）、`scl::hash_table`（闭散列）。
8. 数学类型：`scl::vector2 / vector3 / vector4 / vector2i / matrix / quaternion`。**glm 不出模块边界**。
9. 字符串：用 `cat::String`（typedef）或 `scl::string256`。**禁止**接口里出现 `std::string`。
10. 整型别名：`uint`、`uint8/16/32/64`、`int8/16/32/64`、`byte` 来自 `scl/type.h`。

## 3. 命名

11. 类：PascalCase（`Object`、`Mesh`、`VulkanRender`、`MainGUI`）。
12. 函数与变量：camelCase（`loadNode`、`setMove`、`m_isInit`、`vertexBuffers`）。
13. 枚举值与宏：`ALL_CAPS_WITH_UNDERSCORE`（`PRIMITIVE_TYPE_POINTS`、`KEY_FRAME_TYPE_INVALID`、`MAX_OBJECT_COUNT`、`FIX_COMPONENT_TYPE_TRANSFORM`、`SHADER_PATH`）。
14. 成员前缀：
    - 普通成员：`m_xxx`
    - 静态成员：`s_xxx`
    - 私有/文件局部函数：`_xxx`（如 `_loadVertex`、`_invalidateView`）
15. 接口/特殊前缀：
    - 纯虚接口类用 `I` 前缀：`IRender`、`IFileProvider`
    - simplevulkan POD 结构体用 `svk` 前缀：`svkDevice`、`svkImage`、`svkBuffer`
16. 命名空间：引擎代码全部位于 `namespace cat`；编辑器配置位于 `namespace game`；UI 扩展位于 `namespace imguiex`；YAML 包装位于 `namespace yaml`。
17. **按属性查找统一用 `xxByY` 形式**，不要混用 `findXxxByY` / `getXxxByY` 等同义前缀。标准命名：`objectByID` / `objectByName` / `childByID` / `childByName`（见 `Scene::*` / `Object::*`）。新增同类查询沿用，不从第三方库（imgui 的 `FindWindowByName` 等）借形；第三方代码在 `free/` 下保留各自风格，不在本规则约束内。
18. **简洁服从一致**：能复用已有命名模式时选最短形式（`objectByName` 优于 `findObjectByName`）；当且仅当短名字会与已有重载冲突或暗示错误代价（典型反例：`child(int)` O(1) 与 `child(const char*)` O(N×D) 撞名），才拆成更长的差异化名字（`child(int)` + `childByName(const char*)`）。

## 4. 缩进、对齐、大括号

19. 用 **Tab** 缩进，不要用空格混入。
20. 大括号 Allman / BSD 风：`{` 与 `}` 各自单独占一行。
21. 单语句 if / for 可省略大括号：`if (NULL == ptr) return;`。多语句必须加大括号。
22. **纵向对齐是 cat 的标志风格**，必须保留：
    - 类成员声明列对齐（类型列 / 名字列 / 注释列）
    - 构造函数 init list 列对齐（成员名右侧用空格补齐到统一列）
    - 头文件中的方法列表列对齐（返回类型列 / 函数名列）
    - 参考样例：`object.h` 的成员函数声明、`camera.cpp` 的 init list、`primitive.cpp` 的 init list、`vulkanRender.h` 的 IRender 实现列。
23. 简单 getter / setter 在头文件内联实现（一行）：
    `int id() const { return m_id; }`

## 5. 头文件与包含

24. 用 `#pragma once`，**不要**写 `#ifndef ... #define ... #endif` include guard。
25. include 顺序：
    1. 自身模块头（`#include "cat/xxx.h"`）
    2. `scl/*.h`
    3. 第三方（`vulkan/*.h`、`shaderc/*.h`、`cgltf/*.h`、`rapidyaml/*.h` 等）
    4. C/C++ 标准头（`<stdint.h>` 等）
    分组之间留空行。
26. 头文件优先用前置声明（`struct cgltf_node;`、`namespace yaml { class node; }`），减少传染性 include。
27. 头文件函数参数 `const char* const filename`（双 `const`）是普遍风格，新增字符串/路径参数沿用。

## 6. 类与文件结构

28. 类内顺序：`public:` 构造/析构在前 → 公共方法 → 简单 inline getter → `private:` 工具函数 → `private:` 数据成员。
29. 构造函数 **必须**在 init list 里把所有指针成员显式置 `NULL`、所有标志位显式置默认值。析构函数顺序对称，统一用 `safe_delete`。
30. ID 类对象用全局静态 `ObjectIDMap<T>` + `static T* objectByID(int)` 模式（参考 `Object::s_objectIDMap`）。
31. 命名空间结尾必须带注释：`} // namespace cat`，并保留文件末尾的空行。

## 7. 接口隔离（架构强约束）

32. `cat` 模块**不允许** `#include <vulkan/...>` 或 `#include <shaderc/...>` 或 `#include <spirv_cross/...>`。GPU 相关一律走 `IRender*`，资源句柄用 `void*`。
33. `catbase` 不依赖 `cat` / `catvulkan` / `catui` / `catwindows`。
34. `catvulkan` 是 `IRender` 的具体实现层，可见 Vulkan 全部头。
35. `catwindows` 是平台层，可见 Win32 / EGL；`catui` 仅依赖 ImGui + scl。

## 8. EOF 空行（个人硬规则同步）

36. 修改 `.cpp` / `.h` 时，**严禁增删**文件末尾的空行，必须保持文件原 EOF 状态。新建文件按现有同类文件的末尾空行数来写。

## 9. 注释

37. 允许中文注释。
38. 不强制 doxygen；普通函数加一行 `//` 说明意图即可。
39. 既有的注释掉的死代码不要在无关 PR 里清理；只删除你这次 PR 真正应该删的。
40. 不要添加无意义的"复述代码"注释（不要 `// increment counter` / `// return result` 这类）。

## 10. 平台与宏

41. 平台分支用 `SCL_WIN` / `SCL_APPLE` / `SCL_ANDROID`（来自 `scl/type.h`）。
42. Vulkan vs GLES 分支用 `TEST_VULKAN`（`def.h`），`SHADER_PATH` 等已经按它分发，不要在上层重复判断。
43. 通用宏只用 `def.h` 提供的 `safe_delete` / `safe_delete_array` / `countof` / `OFFSET` / `arg_count`，不要再造同名宏。

## 11. 不要做

- 不要把 `nullptr` / `std::vector` / `std::string` / `std::unique_ptr` 引入接口或成员。
- 不要在 `cat` 模块直接使用 Vulkan 句柄。
- 不要在大括号风格、命名前缀、init list 对齐上引入新风格。
- 不要给文件加 license 头部 / 版权块（现有文件没有，加了会突兀；只有少数文件保留作者日期 banner，是可选的）。
- 不要修改 EOF 空行。

## 12. 提交前自检清单

- [ ] 没有 `nullptr`、没有 `delete xxx;`（除非是 `safe_delete` 内部）。
- [ ] 没有 `std::vector` / `std::string` / `std::map` 等出现在 `.h` 或类成员。
- [ ] 没有 `#include <vulkan/...>` 出现在 `cat/`（只允许在 `catvulkan/`）。
- [ ] 类成员、init list 列对齐保留了。
- [ ] 命名前缀（`m_` / `s_` / `_`）正确。
- [ ] 文件末尾空行没改动。
- [ ] 命名空间结尾带 `// namespace xxx` 注释。

## 13. 错误处理 — assert / ensure / 日志

cat 错误处理哲学：debug 期错误立刻中断；release 期分两类——不可继续就 fatal，可继续就尝试继续。详细对照与改进计划见 `doc/todo/log_fatal设计-Unity-UE对照与改进计划.md`。

工具集：

- `assert(expr)` / `assert(false)` — 不可继续。debug `int 3` / release `throw 1`，**最终终止进程**。
- `ensure(expr)` — 可继续。debug + 调试器附着时 `__debugbreak`，release 写日志，**永远返回 bool 让 caller 决定**。每个 callsite 只触发一次（按 file+line 去重）。
- `log_verbose / debug / info / warn / error / fatal` — 仅日志，无中断；`log_fatal` 是 scl 的 verbosity 等级，**不会终止**。

判别原则（单条够用 90% 场景）：

> 想象 release 下违反这条：
> - 当前调用栈上**能定义出合理的兜底**（早退、跳过、用默认值），且兜底后进程状态仍有意义 → `ensure`
> - 没有兜底，或继续走会让状态更糟（UB、数据损坏、级联崩） → `assert`

速查：

| 错误来源 | 工具 |
|---|---|
| 外部数据（gltf、yaml、shader 编译、网络包） | `ensure` |
| 用户/编辑器交互（ID 找不到、操作失败） | `ensure` |
| 单帧 / 单 draw 上限超出（MAX_DRAW、MAX_JOINT、CB 满） | `ensure` |
| 可选优化路径失败（descriptor cache 满、pipeline 创建失败） | `ensure` |
| 程序内部不变量（链表头 NULL、size 为负、双 free、状态机非法转移） | `assert` |
| 初始化阶段失败（Vulkan device、必需 shader、主 window） | `assert` |
| switch default 真不可能的分支 | `assert` |
| 内存/句柄破坏（必需 new 失败、Vk handle 为 NULL） | `assert` |

写法：

- 推荐 ✅ `if (!ensure(...)) return ...;` 显式早退（多数场景）
- 推荐 ✅ `if (ensure(NULL != ptr)) ptr->do_something();` 通过时执行
- 可用 ✅ `ensure(...);` 不接返回值——仅当 caller 真的不需要走兜底路径，仅作"debug 期暴露 + release 期留日志"的纯检查（debug 期调试器附着仍会 break，不会被漏掉）
- 避免 ❌ `assert(ensure(...));` 双层断言自我矛盾

模糊地带默认 `ensure`：可升级（包一层 `if (!ensure) { assert(false); }`），不可降级；debug 期暴露能力对等；release 宁可丢一帧也别崩编辑器。

反向防滥用：写 `if (!ensure(...))` 之前问自己一句"如果删掉 ensure 让条件违反静默 return，会发生什么？"——答得出"丢一个物体 / 跳过这次操作"就用 ensure；答不出或答"数据结构半初始化"就改 `assert`。

日志策略：

- `assert` 失败 → `scl::assert_write` 三路写（OutputDebugString + printf + urgency_log）—— 崩前必须落盘
- `ensure` 失败 → `scl::log::out(LOG_LEVEL_ERROR, ...)` 单路写 —— 软失败走正常通道即可

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

1. 所有新建文档都必须放在相对当前仓库根目录的 `..\pw\ue\.doc` 目录下面，除非 @caolei 明确指定其他位置。
2. 所有 Markdown 文档（`*.md`）都必须使用中文文件名，文件名要描述主题，不使用 `plan.md`、`design.md`、`document.md`、`checklist.md` 这类泛用英文名。**例外**：目录入口文件 `README.md` 作为 GitHub / 编辑器自动识别的导航文件保留英文名（本身是惯例名而非泛用名）；其它任何"看似导航"的文件不享受此豁免。
3. 修改 `.cpp` 和 `.h` 源文件时，千万不要增加或删除文件末尾的空行；必须保持文件原本的 EOF 空行状态。
4. 不要把个人 skill、个人 agent 配置提交到仓库，除非明确决定团队共享。
5. 代码要在保持可读性的情况下，追求简洁。
6. 不要主动运行 build / 编译验证，除非 @caolei 明确要求；默认只做静态检查、diff 检查或用户指定的轻量验证。
7. 做 review 或 review-driven fix 时，先参考 `doc\review\代码审查-反思-过度防御与冗余修改.md`；不要把理论风险、Warning 或与当前单线程架构不匹配的防御性建议自动升级为必须修改，修复应优先选择最小、直接、符合项目约定的方案，避免引入 mutex / atomic / 冗余判空 / 全局行为改变这类过度防御。

## 说明

`AGENTS.md` 是仓库公共的 agent 入口文件，给 Cursor / Codex / 其他兼容 `AGENTS.md` 协议的工具自动加载。如果存在同目录下的 `AGENTS.override.md`，以 override 文件为准。`AGENTS.md` 与 `AGENTS.override.md` 这两个文件名为了工具识别保留固定英文名，不受"中文文件名"规则约束。
