# 阶段06-子审查-Jolt与构建

子审查 agent：Pascal `019e7e93-23ed-7221-8b9a-71b1858d2fb9`

## Critical

### C1 build_jolt 的 CMake `-A x64` 参数作为单个 argv 传入，失败后仍显示 complete

- commit：`aebb381`
- 文件：`tool/script/setup.py:68` `build_jolt()`，`tool/script/mytool.py:40`，`testCat/CMakeLists.txt:129`
- 风险：`G.arch_param` 在 VS 路径下是单个 argv：`"-A x64"`，不是 `"-A", "x64"`；`subprocess.run(list)` 不会拆分它，CMake 很可能解析失败。失败后 `exec_cmd()` 只打印不抛出，`build_jolt.bat` 仍会走到 `complete`，但 `testCat` 已硬链接 `free/lib64/jolt(_d).lib`，新环境会在生成 / 链接阶段缺库。
- 建议：把 arch 参数改为 list 并展开，例如 `["-A", "x64"]`；`exec_cmd` 失败后重新抛异常；`build_jolt()` 末尾显式检查 `free/lib64/jolt.lib` 和 `jolt_d.lib` 是否存在。

## Warning

### W1 Jolt 构建与链接路径只有 Windows `.lib` 模式

- commit：`aebb381`
- 文件：`tool/build_jolt.bat:3`，`tool/script/setup.py:68`，`testCat/CMakeLists.txt:129`
- 风险：当前 Jolt 构建和链接路径是 Windows `.lib` 模式：`cmake.exe`、VS/Ninja-Windows、`free/lib64/jolt.lib`。没有 Android `arm64-v8a/libjolt.a`、iOS `.a/.xcframework` 或平台分支；如果后续 catphysics / Android / iOS 直接复用这套路径，会配置或链接失败。
- 建议：若只支持 Win64，文档和 CMake 加 `WIN32` guard；若要移动端接入，补 NDK / Xcode 构建入口和 ABI 分目录，例如 `free/lib/android/arm64-v8a/libjolt.a`、`free/lib/ios/libjolt.a`。

### W2 free.7z 含生成物与个人 IDE 状态

- commit：`1122b21`
- 文件：`tool/script/bundle.py:49`，`archive/free.7z`
- 风险：`bundle.py` 只排除 `build/build32/build64`，但实际 `free.7z` 里仍包含多套 `build64_visualstudio`、`CMakeFiles`、`.sln/.vcxproj`，还包含 `*.xcodeproj/xcuserdata/caolei.xcuserdatad`。这会引入生成物和个人 IDE 状态，导致 archive 体积、可复现性和隐私噪音问题。
- 建议：新增排除 `-xr!build*_visualstudio`、`-xr!build*_ninja`、`-xr!CMakeFiles`、`-xr!*.xcuserdatad`、`-xr!xcuserdata`，重新打包 `free.7z`。

### W3 Win64 Jolt 强制 AVX2/F16C/FMADD

- commit：`27138d8`
- 文件：`free/jolt/CMakeLists.txt:34`，`free/jolt/Jolt/Jolt.cmake:625`
- 风险：Win64 Jolt 默认强制 `USE_AVX/AVX2/F16C/FMADD=ON`，MSVC 会走 `/arch:AVX2`。如果 PC 目标不限定 AVX2 CPU，运行时可能非法指令崩溃；这对 Windows 兼容性是 ABI / 发行基线问题。
- 建议：明确 PC 最低 CPU 要求；否则默认降到 SSE2 / SSE4.2，或拆 AVX2 专用库并做运行时 dispatch。

## Suggestion

### S1 Jolt 文档仍混有旧路径和旧集成方式

- commit：`aebb381`
- 文件：`doc/Jolt接入实施步骤.md:205`，`doc/物理引擎接入设计与实施计划.md:107`，`doc/子系统-第三方依赖.md:138`
- 风险：文档仍混有旧说法：`free/JoltPhysics/Jolt`、`add_subdirectory`、`add_dependencies(... Jolt)`；实际实现是 `free/jolt` 独立预构建，`testCat` 只链接 `.lib`，不包含 Jolt target。后续按文档接入容易加错依赖。
- 建议：统一成“预构建 link-only”描述；若未来改回 `add_subdirectory`，再同步 CMake 和文档。

## 补充

- `0b7e932` 的 Jolt 阅读 / 调试指南抽查关键行号引用，未发现会影响构建或平台兼容的明确问题。
- 残余风险：`27138d8` 是 10 万行级 vendored Jolt 源码导入，本轮只做平台宏、CMake、脚本、archive 和文档一致性静态审查，未逐行审 vendor 内部算法。
