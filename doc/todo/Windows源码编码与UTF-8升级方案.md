# Windows 源码编码与 UTF-8 升级方案

## 起源

讨论起点：定稿 `AGENTS.md` 时一开始误写"含中文 cpp/h 必须 UTF-8 with BOM"，被作者质疑。调查仓库后发现：

- `cat/` 下 42 个 cpp/h **全部 UTF-8 无 BOM**，包括含中文注释的 7 个文件
- 各模块 CMakeLists 已经统一加了 `add_definitions("/source-charset:utf-8")`
- 这套做法实际上与 **UE5 的方向暗合**（UE4 大量 BOM，UE5 大清理后 `/utf-8` 无 BOM）

由此引出三个层次的工程问题：
1. cat 自身要不要从 `/source-charset:utf-8` 升级到完整的 `/utf-8`？
2. scl 基础库里 `_ANSI` 宏的 GBK 假设要不要演化？
3. 中文 Windows 环境下，行业到底怎么处理 narrow `char*` 编码问题？

---

## 问题本质：Windows 上 narrow `char*` 的"三国杀"

Windows 上 `char*` 字面量长期处于**三种语义并存**的状态：

| 语义 | 现实场景 |
|---|---|
| A. `char*` = 系统 ANSI codepage（中文 Windows = GBK/CP936） | 老 Win32 ANSI API、ANSI 控制台、`OutputDebugStringA`、CRT `fopen` |
| B. `char*` = UTF-8 | 现代 OSS 库、JSON/YAML/HTTP、Linux/Mac 默认 |
| C. `char*` 不承担文本，靠 `wchar_t` 才精确 | UE/Qt 的 `TCHAR`/`QString` 抽象 |

- Linux/Mac/Android 已经统一到 **B**
- Windows 历史包袱导致 **A、B、C 三者并存**
- scl 的 `_SCL_ENCODING_GBK_` 开关本质上是在 A 和 B 之间手动二选一

### MSVC 三个编译选项的关系

| 选项 | 作用 | 解决问题 |
|---|---|---|
| `/source-charset:utf-8` | 源文件按 UTF-8 解析 | C4819 警告、源码读入乱码 |
| `/execution-charset:utf-8` | 字符串字面量在 `.exe` 里按 UTF-8 存储 | 字符串字面量字节布局 |
| `/utf-8` | 上面两条的合写 | 两者一起 |

`/utf-8` 解决的是"源码读入"和"二进制存储"两段，**不能改变 Win32 ANSI API、不能改变 Console codepage、不能改变 CRT 语义**——这些边界仍然在 A 的世界。所以"内部 UTF-8 + 边界转换"是绕不开的现实，`_ANSI` 这种边界胶水不会消失。

**问题不是要不要保留 `_ANSI`，而是 `_ANSI` 的输入端假设是什么、命名是否暴露这个假设**。

---

## 仓库现状调查

### 1. 源文件编码

| 类别 | 数量 | 备注 |
|---|---|---|
| `cat/` 下 cpp/h 总数 | 42 | |
| UTF-8 with BOM | 0 | |
| 含非 ASCII（中文注释，合法 UTF-8）| 7 | gltfLoader.cpp/h、primitive.cpp/h 等 |
| 纯 ASCII | 35 | |
| UTF-16 | 0 | |

**结论：100% UTF-8 无 BOM，零混杂**。

### 2. CMake 编译选项

15 个 cat 自家 CMakeLists 全部带了 `add_definitions("/source-charset:utf-8")`：

- 主库：`cat`、`catbase`、`catwindows`、`catvulkan`、`catui`
- 测试：`testCat`、`test/testECS`、`test/testCatbase`、`test/testYaml`、`test/testImgui`、`test/testScl`、`test/testMath`
- 自家三方薄壳：`free/scl`、`free/libimg`、`free/rapidyaml`

生成出来的 vcxproj 也确实带上了：

```56:56:E:\cat\cat\build64\cat.vcxproj
      <AdditionalOptions>%(AdditionalOptions) /source-charset:utf-8</AdditionalOptions>
```

但**只有半个 UTF-8**（缺 `/execution-charset:utf-8`）：
- C4819 警告和源码解析乱码：已彻底解决 ✓
- 字符串字面量 `"中文"` 在 `.exe` 里仍是 GBK 字节 ✗

### 3. 处理位置

**CMake 处理，不是 `tool/script` 也不是手工写 vcxproj**。`tool/script/setup.py` 只调用 `cmake.exe -G "Visual Studio 17 2022"` 把 CMakeLists 翻译成 sln/vcxproj，编译选项是各模块 `CMakeLists.txt` 里注入的。

### 4. 中文字符串字面量分布

| 模块 | 中文字面量 | 中文仅在注释 |
|---|---|---|
| `cat/` | 0 | 全部 |
| `catbase/` | 0 | 全部 |
| `catwindows/` | 0 | 全部 |
| `catvulkan/` | 0 | 全部 |
| `catui/` | 0 | 全部 |
| `test/testScl/` | 多处（含 `_ANSI("中文")`）| - |
| `test/testYaml/quickstart.cpp` | 少量（语言测试用）| - |

**主库零中文字面量、零 `_ANSI` 调用**。

### 5. Win32 ANSI API 调用

虽然代码里有 `OutputDebugStringA`、`GetModuleFileNameA`、`LoadLibraryA` 调用，但全部是：
- 参数纯 ASCII（如 `LoadLibraryA("user32.dll")`）
- 或参数是运行时构造的非字面量

**没有任何 `MessageBoxA("中文字面量")` 这种地雷**。

### 6. `_ANSI` 宏与 GBK 假设

`free/scl/encoding.h` 里：

```20:38:E:\cat\free\scl\encoding.h
#define _ANSI(s) scl::debug_to_ansi(s).c_str()


#define MAX_DEBUG_SOURCE_STRING_LENGTH 128

#ifdef SCL_WIN
inline string<MAX_DEBUG_SOURCE_STRING_LENGTH> debug_to_ansi(char* s)
{
#ifdef _SCL_ENCODING_GBK_
	return s;
#else
	wstring<MAX_DEBUG_SOURCE_STRING_LENGTH> ws;
	ws.from_gbk(s); //由于在windows下，源代码总是gbk编码，所以这里from_gbk
	string<MAX_DEBUG_SOURCE_STRING_LENGTH> ss;
	ws.to_ansi(ss.c_str(), ss.capacity());
	return ss;
#endif
}
#endif
```

- 注释明说"在 windows 下，源代码总是 gbk 编码"
- 当前状态（`/source-charset:utf-8` 但无 `/execution-charset`）下，二进制里 `"测试.txt"` 仍是 GBK 字节，`from_gbk` 正好对上，**所以现在能跑**
- 升级到 `/utf-8` 后，二进制变 UTF-8 字节，`from_gbk` 解码 UTF-8 = **数据腐烂**

调用面：
- 主库 cat/catbase/catwindows/catvulkan/catui：**零调用**
- 测试：`test/testScl/testFile.cpp`（4 处）、`test/testScl/testIniFile.cpp`（~10 处）
- 外部：作者的 Linux 服务器项目也用 `_ANSI`，但在 Linux 上是 pass-through 无副作用

---

## UE5 完整做法

### 1. 字符串类型：拒绝 `char*` 做主类型

- `FString` 内部是 `TArray<TCHAR>`，**Windows 上 TCHAR = wchar_t = UTF-16**（其他平台是 UTF-8 char）
- 应用层几乎不直接处理 `char*`，全部用 `FString` 和 `TEXT("中文")`
- `TEXT("中文")` 在 Windows 上展开为 `L"中文"`——**宽字符字面量**，与 source/execution charset 都无关
- 引擎层不依赖 `/utf-8` 选项就能正确处理中文字面量

### 2. 编译选项：UE5 默认 `/utf-8`，UE4 靠 BOM

- UE5 在 `UnrealBuildTool/Platform/Windows/VCToolChain.cs` 默认加 `/source-charset:utf-8 /execution-charset:utf-8`
- UE4 历史代码一堆 BOM 文件，UE5 大清理过一轮

### 3. 边界：和 Win32 ANSI 世界打交道时显式转换

- `StringCast<ANSICHAR>` / `StringCast<WIDECHAR>` 模板做窄/宽互转
- `TCHAR_TO_UTF8` / `UTF8_TO_TCHAR` 宏做 UTF-8/平台 char 互转
- **几乎不调 A 版本 Win32 API**：CreateFile 一律走 W 版本
- Console / Debug 输出：`UE_LOG` 内部转 wchar 写 `OutputDebugStringW`

### 4. 不依赖运行时 locale

- UE 的关键设计选择：**不依赖 `setlocale` / `GetACP`**
- 同一份引擎在中文/英文/日文 Windows 上行为完全一致
- 这跟 scl 的 `_ANSI` 思路**截然相反**：`_ANSI` 是"运行时迁就系统 ANSI"，UE 是"运行时不理系统 ANSI"

### 5. 中文 Windows 用户的体感

- UE5 中文 Windows 用户**几乎感受不到 codepage 问题**
- 这是**"绕开问题"**而不是"解决问题"，代价是引擎全程多一倍内存（UTF-16）和强制 FString 抽象

---

## 其他成功项目对比

| 项目 | 内部字符串 | Windows 上策略 | 跨界做法 | 中文 Windows 痛感 |
|---|---|---|---|---|
| UE5 | FString = wchar (UTF-16) | `/utf-8` + 全 W API | StringCast 显式转换 | 极低 |
| Qt 5/6 | QString = UTF-16 | `/utf-8` + 全 W API | `fromUtf8`/`toUtf8`/`fromLocal8Bit` | 极低 |
| Chromium | std::string = UTF-8 | W API + base::FilePath wstring | `base::UTF8ToWide` | 极低 |
| Rust 标准库 | String = UTF-8 | std::fs 内部转 wchar 调 W API | OS 层封装，user 不感知 | 极低 |
| Boost.Nowide | std::string = UTF-8 | 提供 nowide::ifstream 等替代品 | 替换标准 I/O，不引新字符串类 | 极低 |
| VSCode / ripgrep / bat | UTF-8 everywhere | manifest 设 activeCodePage=UTF-8（Win10 1903+）+ W API | 同上 | 低（Win10+） |
| CMake | std::string = UTF-8 | 内部 `_wopen` + W API | OS 层封装 | 极低 |
| Git for Windows | char* 多种语义混杂 | 受 mingw / libiconv 影响混乱 | 各种 ad-hoc 转换 | 中等，有 bug 历史 |
| Python 3 | str = Unicode (PEP 393) | 全 W API 路径 | encode/decode 显式 | 极低 |

### 共同模式（>90% 的现代项目）

1. **不让 `char*` 同时承担两种 narrow 语义**——要么纯 UTF-8（utf8everywhere 派），要么走 wchar（Qt/UE 派）
2. **不依赖运行时 locale**——`setlocale`/`GetACP` 仅用于诊断，不用于语义判断
3. **Windows 上能走 W 就走 W**——A 版本只在和遗留代码握手时用
4. **跨界转换必须命名显式**——`UTF8_TO_WCHAR`、`fromLocal8Bit`、`UTF8ToWide`，不出现"未标注语义的 `char*`"

### scl `_ANSI` 宏的真正硬伤

不是"假设 GBK"本身，而是**命名没有暴露输入端假设**。读代码的人不知道"输入是 UTF-8 还是 GBK"——这是写在编译期开关里的隐式契约。

---

## 本项目的处境（个性化）

| 维度 | 处境 |
|---|---|
| 工作终端 | 中文 Windows，Console = CP936/GBK |
| 编辑器 | UTF-8 无 BOM（VSCode/Cursor 默认） |
| 主库 cat | 零中文字面量，纯 ASCII + UTF-8 注释 |
| scl 基础库 | 多平台共用，Linux 服务器项目也吃 |
| `_ANSI` 真实用途 | 调试期偶尔传中文给 ANSI 通道（log、控制台、fopen 路径） |

跟 UE 不一样的地方：UE 是个**封闭引擎**，可以强制所有应用走 FString；scl 是**基础工具库**，需要兼容多种调用环境（包括作者自己在 Linux 服务器项目里写 `_ANSI("中文路径")`）。

---

## 分阶段升级方案

### 阶段 1：主库升级 `/utf-8`（零风险，建议立即做）

**范围**：`cat/`、`catbase/`、`catwindows/`、`catvulkan/`、`catui/` 5 个 CMakeLists。

**改动**：`add_definitions("/source-charset:utf-8")` → `add_definitions("/utf-8")`。

**验证**：因为主库零中文字面量、零 `_ANSI` 调用、零 ANSI API 中文参数，**确保不会有任何运行时差异**。

**收益**：
- 字符串字面量字节布局正式切到 UTF-8
- 未来主库加中文字面量（log、错误消息等）即可放心使用
- 与 UE5 标准做法完全对齐

**操作步骤**：
1. 改 5 个 CMakeLists
2. 重跑 `setup.py generate_testCat` / 各模块 generate
3. 编译验证（@caolei 触发，agent 不主动 build）

### 阶段 2：测试模块升级（低风险）

**范围**：`test/testCatbase`、`test/testECS`、`test/testImgui`、`test/testMath`、`test/testYaml` 5 个 CMakeLists。

**改动**：同阶段 1。

**前置检查**：grep 确认这些测试无 `_ANSI("中文")` 调用。`test/testYaml/quickstart.cpp` 有少量中文字面量（"惑星（ガス）"、"行星（气体）"），需要确认这些字面量的接收方是 UTF-8 语义（yaml 库内部一般是 UTF-8）。

### 阶段 3：scl 演化（中风险，必须先做设计）

**前提**：完成 scl 内部的 `_ANSI` 演化（详见下节）。

**范围**：`test/testScl`、`testCat`、`free/scl`、`free/libimg`、`free/rapidyaml` 5 个 CMakeLists。

**风险点**：`test/testScl/testFile.cpp`、`test/testScl/testIniFile.cpp` 的 `_ANSI("中文")` 调用必须在 scl 演化完成后才能正确工作。

**外部依赖**：作者的 Linux 服务器项目也使用 scl 的 `_ANSI` 宏，scl 演化方案必须保证其行为不变。

### 不动的范围

- `free/` 下第三方（jolt、imgui、libpng、libimg、rapidyaml、scl、shaderc、spirv_cross、zlib 等）：他们自己的 CMakeLists 怎么写就怎么写，完全不动
- 但其中 `free/scl/CMakeLists.txt`、`free/libimg/CMakeLists.txt`、`free/rapidyaml/CMakeLists.txt` 是 cat 写的薄壳，属于阶段 3 一并处理

---

## scl 演化方案设计

### 核心思路

把 `_SCL_ENCODING_GBK_` 这个**单维度开关**拆成两个**正交维度**：

| 旧（单一开关） | 新（两个正交开关） |
|---|---|
| `_SCL_ENCODING_GBK_` 一个变量管两件事 | `_SCL_SOURCE_UTF8_`（源码字面量编码）+ `_SCL_RUNTIME_NARROW_*`（运行期 char* 库内语义） |

### 实现草图

```cpp
inline auto debug_to_ansi(const char* s) {
#if defined(_SCL_SOURCE_UTF8_)
    return convert_utf8_to_local_ansi(s);
#else
    return convert_gbk_to_local_ansi(s);
#endif
}
```

由 build system（CMake）根据是否启用 `/utf-8` 自动注入：

```cmake
if(WIN32)
    add_definitions("/utf-8")
    add_definitions(-D_SCL_SOURCE_UTF8_)
endif()
```

这样 Linux 服务器项目（GCC 默认 UTF-8 源码，不依赖 `/utf-8` 选项）也能根据自己的 build system 选择。**核心是：库的行为根据 build flag 自动适配，使用方零代码改动**。

### 命名升级

`_ANSI` 命名是债，建议未来逐步迁移到三个意图明确的宏：

```cpp
_TO_LOCAL_NARROW("中文")   // 输出"系统 ANSI"，给老 API（Windows GBK / Linux UTF-8）
_TO_UTF8("中文")           // 输出 UTF-8，给现代库
_TO_WIDE("中文")           // 输出 wchar，给 Win32 W API
```

旧 `_ANSI` 做成 `_TO_LOCAL_NARROW` 的 alias，标记 deprecated，慢慢替换。

### 哲学方向

**utf8everywhere 派**对本项目更友好（与 Qt/UE 的 wide 派相比）：

- 源码 UTF-8 无 BOM ✓ 已做
- 阶段 1 升 `/utf-8` 后字符串字面量也 UTF-8 ✓
- scl 内部 char* 默认就是 UTF-8 ✓ 当前未定义 `_SCL_ENCODING_GBK_`
- 只有 `_ANSI` 这个出界胶水需要演化

---

## 总结

> 这不是"GBK 还是 UTF-8"的二选一，而是"**让运行时去 setlocale 兼容** vs **让 build 期编译期固定语义**"的两种世界观之争。前者是 90 年代遗产（scl 的 `_ANSI` 是这条路），后者是 2010 后行业共识（UE5/Qt/Chromium/Rust 全在这条路）。
>
> cat 主库已经在走对了，scl 只是历史包袱需要慢慢消化。

---

## 决议（截至定稿）

- 阶段 1 / 2 / 3 **暂不执行**，先有此文档
- AGENTS.md 已经按"统一 UTF-8 无 BOM"口径定稿
- scl 演化方案需要单独立项讨论后再动
