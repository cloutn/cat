# Jolt 接入实施步骤

> 上级：[物理引擎接入设计与实施计划](./物理引擎接入设计与实施计划.md)
> 关联：[子系统-第三方依赖](./子系统-第三方依赖.md)

本文是把 Jolt Physics 5.5.0 接入 cat 引擎的具体落地步骤。已经决定的设计选项见第 1 节；版本登记规则、目录改造、CMake 修改、风险点见第 2~5 节。

## 1. 决策摘要

| 项 | 决定 |
|----|------|
| 目录命名 | `free/jolt`（全小写，对齐 `imgui / rapidyaml / spirv_cross` 风格）|
| 源码组织 | **保留** Jolt 上游 `Jolt/` 子树原样；**保留**上游 `Build/CMakeLists.txt`；cat 写薄壳转发 |
| 版本登记 | **不引入** `THIRD_PARTY.md`；每个第三方库下放 `version.txt`（一行版本号）|
| `OBJECT_LAYER_BITS` | 32 |
| `DOUBLE_PRECISION` | OFF（与 UE / Unity 一致，cat 的 `scl::vector3` 也是 float）|
| `DEBUG_RENDERER_IN_DEBUG_AND_RELEASE` | ON（后续 `PhysicsScene::debugDraw` 直接桥到 `IRender`）|

## 2. 各第三方库版本号（已查证）

我用源码里的版本宏 / 版本文件挖了一遍，**绝大多数库都能精确确认**。完整列表如下，建议在每个库根目录新建 `version.txt`，单行写版本号：

| 库 | 路径 | 版本 | 来源 |
|----|------|------|------|
| imgui | `free/imgui/` | `1.89-WIP (build 18833)` | `imgui.h:25` `IMGUI_VERSION = "1.89 WIP"`, `IMGUI_VERSION_NUM = 18833` |
| rapidyaml | `free/rapidyaml/` | `0.7.2` | `c4/yml/version.hpp:6` `RYML_VERSION = "0.7.2"` |
| glm | `free/glm/` | `0.9.9.9` | `glm/detail/setup.hpp:11` `GLM_VERSION_MESSAGE = "GLM: version 0.9.9.9"` |
| libpng | `free/libpng/` | `1.6.21` | `png.h:285` `PNG_LIBPNG_VER_STRING = "1.6.21"` |
| zlib | `free/zlib/` | `1.2.6` | `zlib.h:40` `ZLIB_VERSION = "1.2.6"` |
| jpeg_turbo | `free/jpeg_turbo/` | `~2.0.x（2019-era）` | `jversion.h` `JCOPYRIGHT` 注明 `2009-2019 D. R. Commander`，对应 libjpeg-turbo 2.0 系列；精确小版本需要看 `package-version` 等文件，不在常规位置 |
| Vulkan headers | `free/vulkan/Include/` | `1.2.131` | `vulkan_core.h:47` `VK_HEADER_VERSION 131` |
| SPIR-V | `free/vulkan/Include/` | `1.5` | `spirv.h:56` `SPV_VERSION 0x10500` |
| **Jolt**（本次接入） | `free/jolt/` | `5.5.0` | 下载来源、`Build/CMakeLists.txt:3` `project(JoltPhysics VERSION 5.5.0 ...)` |
| spirv_cross | `free/spirv_cross/` | 待查 | 需要看 git log 或上游 release tag；源文件没有版本宏 |
| libtga | `free/libtga/` | 待查 | 多数小项目无版本，可记 `unknown / vendored` |
| libktx | `free/libktx/` | 待查 | 较新版本有 `lib/version.h`，旧版无；先标 `unknown`，等下次升级时填 |
| cgltf | `catbase/cat/cgltf.c` 内嵌 | 待查 | cgltf.h 顶部有 `version` 注释行，可查 |
| shaderc | `free/lib64/shaderc_combined*.lib` | 与 Vulkan SDK 1.2.131 同步 | 二进制库，没有源码可读，按 SDK 版本回填 |

> 结论：**版本号不难找**。所有"严肃维护"的库（imgui / rapidyaml / glm / libpng / zlib / Vulkan / Jolt）都有版本宏；个别小工具库（libtga / libktx 旧版）找不到精确版本时，`version.txt` 写 `unknown / vendored YYYY-MM` 即可，至少有一个时间锚点。

`version.txt` 格式建议：

```
5.5.0
upstream: https://github.com/jrouwe/JoltPhysics/releases/tag/v5.5.0
license:  MIT
imported: 2026-05-30
```

第一行是机器可读的纯版本号；后面三行是注释信息，方便日后追溯。

## 3. Jolt 改造步骤

### 3.1 重命名目录

```
free/JoltPhysics-5.5.0/  →  free/jolt/
```

PowerShell：

```powershell
Rename-Item E:\cat\free\JoltPhysics-5.5.0 jolt
```

### 3.2 删除非必要子目录与文件

cat 引擎只用 Jolt 核心库，下面这些是上游样例 / 工具 / 文档 / CI 配置，**全部删除**（约 70 MB）：

| 删除项 | 用途 | 删除原因 |
|--------|------|----------|
| `HelloWorld/` | 上游 hello world 示例 | cat 自己写 demo |
| `Samples/` | 上游 demo 集合 | 同上 |
| `JoltViewer/` | 物理回放工具 | 不集成 |
| `PerformanceTest/` | 性能测试 | 不集成 |
| `TestFramework/` | 上游测试框架 | 不集成 |
| `UnitTests/` | 上游单测 | 不集成 |
| `Assets/` | 测试资源 | 不集成 |
| `Docs/` | 上游文档 | 已有官网 |
| `.github/` | 上游 CI | 不需要 |
| `Doxyfile` `run_doxygen.bat` `sonar-project.properties` | 文档/质量工具 | 不需要 |
| `Build/cmake_*.bat` `Build/cmake_*.sh` `Build/macos_*.sh` `Build/ubuntu*.sh` `Build/mingw-w64-*.cmake` | 上游各平台引导脚本 | cat 用自己的 CMake 入口 |
| `.clang-format` `.editorconfig` `.gitattributes` `.gitignore` | 上游编辑器/git 规则 | 与 cat 风格冲突 |
| `ContributorAgreement.md` | 上游 CLA | 与 cat 无关 |

**保留**：

```
free/jolt/
├── Jolt/                      ← 全部源码与子目录，原样保留
├── Build/CMakeLists.txt       ← 上游主 CMake，原样保留
├── LICENSE                    ← MIT 许可证全文
├── README.md                  ← 上游 README，方便看清楚自己用的是什么
├── version.txt                ← ★ 新建（第 3.3 节）
└── CMakeLists.txt             ← ★ 新建（第 3.4 节）
```

### 3.3 新建 `free/jolt/version.txt`

```
5.5.0
upstream: https://github.com/jrouwe/JoltPhysics/releases/tag/v5.5.0
license:  MIT
imported: 2026-05-30
```

### 3.4 新建 `free/jolt/CMakeLists.txt`（cat 风格薄壳）

```cmake
cmake_minimum_required(VERSION 3.22)

set(target jolt)
project(${target})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----------------------------------------------------------------------
# Jolt 编译选项：锁定 cat 期望的 ABI
# 任何选项变化都会改变 ABI，必须和 testCat 链接侧严格一致。
# ----------------------------------------------------------------------

# 关键：MSVC 运行时库必须用 DLL 版（/MD），与 cat 其他库一致
# Jolt 默认 USE_STATIC_MSVC_RUNTIME_LIBRARY=ON 会用 /MT，会导致 LNK2005
set(USE_STATIC_MSVC_RUNTIME_LIBRARY      OFF CACHE BOOL ""   FORCE)

# 精度 / 异常 / RTTI / 共享库
set(DOUBLE_PRECISION                     OFF CACHE BOOL ""   FORCE)
set(CPP_EXCEPTIONS_ENABLED               OFF CACHE BOOL ""   FORCE)
set(CPP_RTTI_ENABLED                     OFF CACHE BOOL ""   FORCE)
set(BUILD_SHARED_LIBS                    OFF CACHE BOOL ""   FORCE)

# 调试与断言
set(USE_ASSERTS                          ON  CACHE BOOL ""   FORCE)
set(GENERATE_DEBUG_SYMBOLS               ON  CACHE BOOL ""   FORCE)
set(FLOATING_POINT_EXCEPTIONS_ENABLED    OFF CACHE BOOL ""   FORCE)

# Layer / 求解器位宽
set(OBJECT_LAYER_BITS                    32  CACHE STRING "" FORCE)

# SIMD（PC x64 主线，Snapdragon 660 起步的 ARM 走 ARMNeon 路径，不在此 CMake 文件覆盖）
set(USE_SSE4_1                           ON  CACHE BOOL ""   FORCE)
set(USE_SSE4_2                           ON  CACHE BOOL ""   FORCE)
set(USE_AVX                              ON  CACHE BOOL ""   FORCE)
set(USE_AVX2                             ON  CACHE BOOL ""   FORCE)
set(USE_AVX512                           OFF CACHE BOOL ""   FORCE)
set(USE_LZCNT                            ON  CACHE BOOL ""   FORCE)
set(USE_TZCNT                            ON  CACHE BOOL ""   FORCE)
set(USE_F16C                             ON  CACHE BOOL ""   FORCE)
set(USE_FMADD                            ON  CACHE BOOL ""   FORCE)

# DebugRenderer / Profiler
set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE  ON  CACHE BOOL ""   FORCE)
set(DEBUG_RENDERER_IN_DISTRIBUTION       OFF CACHE BOOL ""   FORCE)
set(PROFILER_IN_DEBUG_AND_RELEASE        OFF CACHE BOOL ""   FORCE)
set(PROFILER_IN_DISTRIBUTION             OFF CACHE BOOL ""   FORCE)

# 容器 / 序列化
set(USE_STD_VECTOR                       OFF CACHE BOOL ""   FORCE)
set(ENABLE_OBJECT_STREAM                 ON  CACHE BOOL ""   FORCE)
set(DISABLE_CUSTOM_ALLOCATOR             OFF CACHE BOOL ""   FORCE)

# 编译质量与确定性
set(OVERRIDE_CXX_FLAGS                   OFF CACHE BOOL ""   FORCE)
set(ENABLE_ALL_WARNINGS                  OFF CACHE BOOL ""   FORCE)
set(CROSS_PLATFORM_DETERMINISTIC         OFF CACHE BOOL ""   FORCE)
set(INTERPROCEDURAL_OPTIMIZATION         OFF CACHE BOOL ""   FORCE)

# 安装与跟踪：不需要
set(ENABLE_INSTALL                       OFF CACHE BOOL ""   FORCE)
set(TRACK_BROADPHASE_STATS               OFF CACHE BOOL ""   FORCE)
set(TRACK_NARROWPHASE_STATS              OFF CACHE BOOL ""   FORCE)

# ----------------------------------------------------------------------
# 让上游 Build/CMakeLists.txt 接管 600+ 文件的清单
# 上游 Jolt/Jolt.cmake 中创建的 target 名是 "Jolt"（大写）
# ----------------------------------------------------------------------
set(PHYSICS_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Build)

# ----------------------------------------------------------------------
# 把上游 Jolt target 的输出对齐到 cat 风格
#   - 文件名：jolt.lib / jolt_d.lib（OUTPUT_NAME=jolt + DEBUG_POSTFIX=_d）
#   - 输出目录：free/lib64/
# ----------------------------------------------------------------------
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH 64)
endif()

set_target_properties(Jolt PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/../lib${ARCH}
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../lib${ARCH}
    OUTPUT_NAME                      jolt
    DEBUG_POSTFIX                    _d
)

# 让 cat 上层只用 #include <Jolt/Physics/PhysicsSystem.h> 风格 include
target_include_directories(Jolt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DUNICODE -D_UNICODE)
```

> 注：上游 target 名是 `Jolt`（大写，由 `Jolt/Jolt.cmake` 第 469 行 `add_library(Jolt ...)` 创建），但**输出文件名**通过 `OUTPUT_NAME=jolt` 改成小写。`testCat` 那一侧的 `add_dependencies(... Jolt)` 用 target 名（大写），`target_link_libraries` 链接的 .lib 文件名是 `jolt.lib / jolt_d.lib`（小写）。

### 3.5 改 `testCat/CMakeLists.txt`

> Jolt 走 **shaderc 同款"独立预 build"路径**——由 `tool/script/setup.py:build_jolt()` 在 setup 阶段产出 `free/lib64/jolt(_d).lib`，testCat 只 link 不 add_subdirectory。理由：Jolt 600+ 文件如果 add_subdirectory 进来会把 testCat sln 拖慢、IntelliSense 索引变重、clean build 反复重编 Jolt。

只需 1 处改动：在 `debug_links` 与 `release_links` 末尾各加一行：

```cmake
# debug_links 末尾
debug ${PROJECT_SOURCE_DIR}/../free/lib${ARCH}/jolt_d.lib

# release_links 末尾
optimized ${PROJECT_SOURCE_DIR}/../free/lib${ARCH}/jolt.lib
```

**不要**加 `add_subdirectory(... free/jolt ...)`，**不要**在 `add_dependencies(...)` 列表里加 `Jolt`（这两个属于 add_subdirectory 模式才需要）。

### 3.5.1 在 `tool/script/setup.py` 加 `build_jolt()`

参照已有的 `build_shaderc()` 模式，新增：

```python
def build_jolt():
    arch = "64" if G.arch64 else ""
    src_path = "../free/jolt/"
    build_path = f"../free/jolt/build{arch}_{G.build_suffix}/"
    if not os.path.exists(build_path):
        os.makedirs(build_path)
    exec_cmd(['./cmake/bin/cmake.exe', "-G", G.generator, G.arch_param, "-Wno-dev", "-S", src_path, "-B", build_path])
    exec_cmd(['./cmake/bin/cmake.exe', "--build", build_path, "--config", "Debug"])
    exec_cmd(['./cmake/bin/cmake.exe', "--build", build_path, "--config", "Release"])
```

不需要 `shutil.copy`，因为 cat 写的薄壳 `free/jolt/CMakeLists.txt` 已经把 `ARCHIVE_OUTPUT_DIRECTORY_*` 设到了 `free/lib64/`，build 完直接落到位。

`setup.py` 的 `build_all()` 会自动捕捉所有 `build_*` 函数，因此不用改 `all()`。

### 3.5.2 单跑入口 `tool/build_jolt.bat`

为了和 `tool/generate_testCat.bat` 一致，加一个：

```bat
@echo off
"./python/python" "script/setup.py" build_jolt -arch=64
echo complete.
pause
```

升级 Jolt 时双击它单独跑就行，不必每次 `setup.bat` 全量。

### 3.5.3 手工试运行（不走 setup.py 的快路径）

第一次想验证薄壳 CMake 写得对不对，直接在仓库根开 cmd 跑：

```cmd
.\tool\cmake\bin\cmake.exe -G "Visual Studio 17 2022" -A x64 -Wno-dev -S free\jolt -B free\jolt\build64_visualstudio
.\tool\cmake\bin\cmake.exe --build free\jolt\build64_visualstudio --config Debug
.\tool\cmake\bin\cmake.exe --build free\jolt\build64_visualstudio --config Release
```

跑完应当看到 `free\lib64\jolt.lib` 和 `free\lib64\jolt_d.lib`。

### 3.6 给现有第三方库补 `version.txt`（顺手做）

按第 2 节的查证结果，给以下库各建一个 `version.txt`：

| 路径 | 第一行内容 |
|------|-----------|
| `free/imgui/version.txt` | `1.89-WIP-18833` |
| `free/rapidyaml/version.txt` | `0.7.2` |
| `free/glm/version.txt` | `0.9.9.9` |
| `free/libpng/version.txt` | `1.6.21` |
| `free/zlib/version.txt` | `1.2.6` |
| `free/jpeg_turbo/version.txt` | `2.0.x-2019` |
| `free/vulkan/version.txt` | `1.2.131` |
| `free/spirv_cross/version.txt` | `unknown` |
| `free/libtga/version.txt` | `unknown` |
| `free/libktx/version.txt` | `unknown` |
| `free/jolt/version.txt` | `5.5.0` |

`version.txt` 不参与构建，纯文本登记，便于 grep / 排查升级影响。

### 3.7 更新 `doc/子系统-第三方依赖.md`

- §1 表"依赖一览"末尾追加：

  ```
  | jolt | free/jolt/ | Jolt Physics 5.5.0，物理引擎；通过 catphysics 接入 |
  ```

- §3 新增小节 "3.X jolt"：写上保留上游目录树 + 转发 `Build/CMakeLists.txt` + 关键 option。
- §4 构建顺序图末端加 `→ jolt → testCat`。

## 4. 关键风险点

按风险高低排序，挨个说一下，避免后续踩雷：

### 4.1 ★★★ MSVC 运行时库错配（必踩）

- **现象**：链接期 `LNK2005`、`LNK4098 'MSVCRT' conflicts with use of other libs`，或运行期 `_CrtIsValidHeapPointer` 断言。
- **原因**：cat 全部库默认走 DLL CRT（`/MD`、`/MDd`）；Jolt 上游默认 `USE_STATIC_MSVC_RUNTIME_LIBRARY=ON`，走 `/MT`、`/MTd`。两者混链必崩。
- **对策**：薄壳 CMake 第一行选项 `set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)`，已写在第 3.4 节。
- **验证**：链接成功后用 `dumpbin /directives jolt.lib | findstr DEFAULTLIB` 检查应包含 `MSVCRT` / `MSVCRTD`，不应包含 `LIBCMT`。

### 4.2 ★★★ Jolt target 名是大写 `Jolt`，cat 链接产物期望小写 `jolt.lib`

- **冲突**：cat 现有约定全小写，但上游 `add_library(Jolt ...)` 写死。
- **对策**：在 `set_target_properties` 里 `OUTPUT_NAME jolt`（已写）。独立预 build 路径下 testCat 只 `target_link_libraries` 链 `jolt.lib / jolt_d.lib`，不引用 target 名，所以这里冲突自动消失。

### 4.3 ★★ 上游 `Distribution` 配置干扰

- **现象**：`Build/CMakeLists.txt:127` 在顶层会追加 `Distribution` 配置；如果我们 `add_subdirectory` 引入它，但不希望生成 `Distribution`。
- **对策**：上游已经用 `if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)` 守住——只在顶层项目时追加，子项目时不追加。所以 `add_subdirectory` 引入是安全的。
- **验证**：CMake 配置后 `cmake -L` 看到的 `CMAKE_CONFIGURATION_TYPES` 仍只有 `Debug;Release`。

### 4.4 ★★ `OBJECT_LAYER_BITS` 与 `DOUBLE_PRECISION` 改后所有 .lib 必须全部重编

- 这两个选项进 Jolt 的所有头文件类型布局，改了之后下游 cat / catphysics 看到的类型大小变了，**不重编 catphysics** 会运行期莫名 corruption。
- **对策**：`version.txt` 加一行注释说明当前 ABI；以后改这两个选项的 PR 必须 clean build 全部依赖 Jolt 的模块。

### 4.5 ★ `DebugRenderer` 启用后不调用初始化也无害

- Jolt 的 `DebugRenderer` 是抽象类，`DEBUG_RENDERER_IN_DEBUG_AND_RELEASE=ON` 只是把抽象基类编进 lib。真正调试可视化要等 catphysics 实现一个 `JoltDebugRendererBridge` 把 `DrawLine / DrawTriangle` 桥到 `IRender`，目前可以先不实现，留给 M3 里程碑。

### 4.6 ★ 关于"大写 Jolt 目录" vs "小写 jolt 目录"的潜在混淆

- 我们把 cat 这边的目录叫 `free/jolt`，但上游内部源码路径是 `free/jolt/Jolt/...`（大写）。
- 这是上游约定（`#include <Jolt/...>`），改不动也不该改——改了等于把 600+ 文件的 include 路径都断掉，每次升级都要重做。
- **结论**：cat 外壳目录小写、上游内部目录大写，两者并存是合理且符合业界惯例的（Bullet 也叫 `bullet3/src/Bullet3*/`，PhysX 叫 `physx/source/...`）。

## 5. 验证 Checklist

按顺序做完，每项打勾：

- [ ] `Rename-Item` 完成，`free/jolt/` 存在，`free/JoltPhysics-5.5.0/` 不存在
- [ ] 删除清单（§3.2）执行完毕，`free/jolt/` 体积 ~30 MB
- [ ] `free/jolt/version.txt` 存在
- [ ] `free/jolt/CMakeLists.txt` 存在，内容如 §3.4
- [ ] `free/jolt/Jolt/` 与 `free/jolt/Build/CMakeLists.txt` 仍在
- [ ] `testCat/CMakeLists.txt` 1 处改动到位（§3.5：只在 link 末尾各加一行 jolt(_d).lib）
- [ ] `tool/script/setup.py` 新增 `build_jolt()`（§3.5.1）
- [ ] `tool/build_jolt.bat` 创建（§3.5.2）
- [ ] 11 个 `version.txt`（§3.6）就位
- [ ] `doc/子系统-第三方依赖.md` 更新（§3.7）
- [ ] 单跑 `tool/build_jolt.bat` 或手工 cmake（§3.5.3）：CMake configure 通过，`CMAKE_CONFIGURATION_TYPES = Debug;Release`（不含 Distribution）
- [ ] Debug build：`free/lib64/jolt_d.lib` 生成
- [ ] Release build：`free/lib64/jolt.lib` 生成
- [ ] 跑 `tool/generate_testCat.bat`，testCat sln 链接通过（debug + release 两个配置）
- [ ] `dumpbin /directives free/lib64/jolt.lib | findstr DEFAULTLIB` 确认走 DLL CRT
- [ ] cat 任意 `.cpp` 写一行 `#include <Jolt/Physics/PhysicsSystem.h>` 不报错（这一步可以放到 catphysics 模块创建时再做）

## 6. 后续

- 本文只覆盖到"链接通"。具体的 `IPhysics / PhysicsScene / PhysicsBody / PhysicsShape / Ray / Raycast` 设计、`catphysics` 模块拆分、`PhysicsComponent` 与 `Object` 绑定、Pick Pass 替换为 Raycast——见 [物理引擎接入设计与实施计划](./物理引擎接入设计与实施计划.md) §3~§5（M0~M5 里程碑）。
- 升级 Jolt 的标准流程（建议成文档之外的小 SOP）：
  1. 下载新版本到任意临时目录
  2. 把 `free/jolt/Jolt/` 整个替换为新版本的 `Jolt/`
  3. 把 `free/jolt/Build/CMakeLists.txt` 替换为新版本的同名文件
  4. 检查薄壳 CMake 里的 option 在新版本是否有重命名 / 新增（diff 一下两份 `Build/CMakeLists.txt` 的 `option(...)` 行）
  5. 更新 `version.txt`
  6. 全量 clean build，跑 `testCat`，过 raycast / overlap 烟测
