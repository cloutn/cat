# [bbbbaa1] fix: 7 P0 crash fixes (shader/env/primitive/descAlloc/yaml)

- 提交时间：2026-05-31 22:22
- 涉及文件：cat/env.cpp, cat/primitive.cpp, cat/shader.cpp, catbase/cat/yaml.cpp, catvulkan/cat/descriptorAllocator.cpp

## Commit 与代码一致性

- 与 commit 描述一致：5 个文件、7 个 P0（env release NULL、primitive draw NULL shader、primitive updateVertices NULL 数组、shader `_loadfile` 越界、yaml `mmap` NULL、descAlloc fast-path 漏 fresh page、descAlloc free 索引未校验）。
- 提交注释（如 `// release 类函数对齐 safe_delete / vkDestroyXxx 语义`、`// 契约：caller 已保证 macros != NULL`）准确描述了语义，且符合仓库《代码审查-反思》中"防御-契约"边界的约定（contract by caller，release-类接 NULL/sentinel）。
- `cat/shader.cpp` 实际落盘文件比 diff 多出更详尽的中文注释（"原 while 越界" / "丢前置注释"），属于格式增强，不改变行为，一致。
- `cat/env.cpp:96` 的 sentinel 比较 `pTextureFile == &TextureFile::empty()` 与 `env.cpp:80` 处 `loadTextureFile` 出错返回 `&TextureFile::empty()` 配对，闭环一致。

## Critical

（无）—— 本次 diff 修的都是真崩溃；逐处复核未发现"修复反引入崩溃"的情况：

- `shader.cpp::_loadfile`：filesize=0 时 `new char[0 + 1]` 合法（size=1），三元分支 `(filesize > 0) ? strstr(...) : NULL` 直接给 `version = NULL`，进入"无 #version"分支，`memcpy(out, macros, macrolen)` + `memcpy(out + macrolen, fileBuf, 0)` 均合法（size 0 的 memcpy 是 well-defined），最后 `out[outlen - 1] = '\0'` 在 `outlen = 0 + macrolen + 1 ≥ 1` 时合法。OK。
- `primitive.cpp::updateVertices`：构造函数确认 `m_deviceVertexBuffers(NULL)`（`primitive.cpp:28`），新增 `NULL == m_deviceVertexBuffers || m_deviceVertexBuffers[0] <= 0` 利用短路求值，先判 NULL 再下标访问，无 UB。
- `descriptorAllocator::_allocFromPages`：原条件把"全新 page（`allocCount = PAGE_SIZE`、`freeIndices.empty()`）"误当作"满"跳过，几乎使整个 fast-path 不可用；新条件 `freeIndices.size() == 0 && allocCount == 0` 才是真"满"，与同文件 `descriptorAllocator.cpp:94` 的 `assert(freeIndices.size() > 0 || allocCount > 0)` 在 `page.alloc()` 入口的契约完全匹配；非满 page 一定能成功 alloc，不会再触发该 assert。

## Warning

- [W1][shader] **UTF-8 BOM 未处理**。若 `.vs/.ps` 文件由编辑器写出带 BOM（`EF BB BF`），`strstr(fileBuf, "#version")` 仍能命中 offset = 3 处的 `#version`，于是 `prefix` 段会变成 `BOM + "#version ... \n"`，最终拼成 `BOM + #version 行 + macros + rest`。多数 GLSL 编译器要求 `#version` 必须严格出现在源文件首行首列（仅允许空白/注释），BOM 前置极易触发 `0:1: error: '#' : invalid directive` 而看不出原因。仓库 AGENTS.md 明确要求"UTF-8 无 BOM"，所以这是约定级风险，但若有第三方/插件 shader 文件携 BOM，本路径会静默错误。建议在 `fileBuf` 读完后 `if (filesize >= 3 && (uint8)fileBuf[0]==0xEF ...) skip3`，或在第一次拼装前显式 trim BOM。
- [W2][shader] **`macros != NULL` 仅靠注释契约**。`Shader::shader()` 当前唯一 caller 传 `String::c_str()`（`shader.cpp:37-38`），`scl::String::c_str()` 通常返回 `""` 而非 `NULL`，目前安全。但 `_loadfile` 是 `private` 成员？若后续有人新增 caller 直接传 NULL，`::strlen(NULL)` 立即 UB / 崩。建议加一行 `assert(NULL != macros)` 把契约"机器化"（不展开为运行时分支，避免回到"双保险防御"反模式），与 commit 注释的"契约"风格一致。
- [W3][descAlloc] **`allocCount` 归零时机**。修复正确性依赖一个隐含不变量："page 在分配过 PAGE_SIZE 次之前，`allocCount > 0` 始终成立"。从 `descriptorAllocator.h:26` + `descriptorAllocator.cpp:97-100`（`if (allocCount > 0) { index = allocCount - 1; --allocCount; }`）看，`allocCount` 由 `PAGE_SIZE` 严格递减到 0，与 `freeIndices.push()` 是互斥来源，所以"`freeIndices.empty() && allocCount == 0`"确实=`满`。但若未来在别处直接清零 `allocCount`（如 `reset()` / 重用 page），就会把"非满"page 当满跳过。建议在 `DescriptorPage` 上加 `bool isFull() const { return freeIndices.empty() && allocCount == 0; }`，把这个语义沉到一处，调用方写 `if (page.isFull()) continue;` 而不是裸条件。
- [W4][primitive] **draw 内统一改用参数 `render`**。原代码非 pick 路径用 `m_shader->shader(m_render)`，pick 路径用 `m_pickShader->shader(render)`；新代码统一为 `targetShader->shader(render)`。如果 `m_render` 与 `render` 一定相等（典型由同一个 IRender 注入），此简化无副作用；但若有调用链通过 `setRender` 在某时刻替换 `m_render`、又把"老 render"传入 `draw`，行为会变化。从同文件 `m_render` 字段语义看，二者应保持一致，建议同 commit 顺手补一行 `assert(m_render == NULL || m_render == render)` 把这个隐性等式锁死，否则后续若有人引入双 render 路径，bug 难定位。
- [W5][env] **sentinel 比较只在"caller 持有的就是同一个 `&empty()`"时生效**。`pTextureFile == &TextureFile::empty()` 是地址比较，前提是 caller 拿到的指针来自 `Env::loadTextureFile` 等内部接口（它们确实返回 `&TextureFile::empty()`，见 `env.cpp:80`）。若 caller 在外部以"等价空对象"传入（例如自建栈上 `TextureFile{}`），比较为 false，会落入下面 `m_textureFiles.find(pTextureFile->name)` 流程；此时 `pTextureFile->name` 是空字符串，map 找不到则 return，仍然不崩，但语义偏移。**当前**无实际风险（caller 唯一在 `material.cpp:39` 传入也是 Env 给出的指针），仅作约定提醒：sentinel 拦截语义建议在 doc 中显式声明"empty 只允许是 `Env::TextureFile::empty()` 同一对象"。

## Suggestion

- [S1][shader] `_loadfile` 返回 `char*` 但调用方 `Shader::shader()` 立即 `delete[]`，且现实路径下两次分配（`fileBuf` + `out`）+ 一次 `memcpy`。如果 shader 编译已成热点，可以考虑：filesize 已知 + macros 已知 → 单次 `new char[outlen]`，直接 `f.read(out + prefixLen, ...)`；本次修复为可读性/安全优先，不必同 commit 推进。
- [S2][shader] `assert(readlen == filesize)` 在 release 下被消除，但 `readlen != filesize` 会导致 `fileBuf[filesize] = '\0'` 之后中段含未初始化字节，最终被原样拼进 `out`。可加 `if (readlen != filesize) { delete[] fileBuf; delete[] out; return NULL; }`（这与 `shader.cpp:41` 已有的 `NULL != vs_code && NULL != ps_code` 守门吻合）。
- [S3][descAlloc::free] 新增的两条 `assert` 仅在 debug 生效。`pageIndex` / `setIndex` 越界在 release 下会写到 `m_pages[bogus].freeIndices`，是 P0 级别的潜在内存损坏。建议 release 下也 `if (pageIndex < 0 || pageIndex >= m_pageCount) return;`（release 类接口对脏 ID 静默忽略，与 env release 风格一致）。
- [S4][yaml] `document::load` 在 `fm.map() == NULL` 时直接 `return node()` 不会泄漏（`scl::filemap fm` 是栈对象，析构 RAII 关闭句柄），处理 OK；建议同函数内统一 early-return 形态：若后续还有任何"读 buffer 前的失败分支"也用同一 `return node();`，保持调用方"空 node 即失败"的语义单一。
- [S5][primitive] `assert(false); return;` 是常见模式，但与仓库《代码审查-反思》一致的写法应是把"违反契约"信息打出来便于排查；可考虑写一个 `CAT_ASSERT_MSG(false, "Primitive::draw without setShaderWithPick")` 之类的宏（不是本 commit 强求项）。

## 影响范围分析

- `cat/shader.cpp::_loadfile` 是 Shader 编译唯一文件源路径，所有 GLSL/SPIRV 生成共用；本次修复直接消除"末行无换行 → `*p++` 越界"在长 shader 文件上的偶发崩，影响面=所有 shader 加载，**受益大**；副作用仅是多一次 `new + memcpy`（与文件 IO 同量级，可忽略）。
- `cat/primitive.cpp::draw` / `updateVertices` 是渲染主循环每帧 N 次调用；新增 `NULL` 守门是 O(1) 分支，CPU 端无成本；但消除了"未 setShader 的 Primitive 进入 draw"导致的解引用崩。
- `cat/env.cpp::releaseTextureFile` 在 `material` 析构与资源卸载路径上，每次卸材质会走一次；接 NULL/sentinel 后，外部 caller 错误传入不再崩，错位释放可由后续日志覆盖。
- `catvulkan/cat/descriptorAllocator.cpp::_allocFromPages` 是 Vulkan descriptor 关键热点：原条件等价于"每帧大概率走 createPage 新页"路径，会持续膨胀 `m_pages`，修复后能正确复用 fresh page，**预计可见的内存/分配抖动收敛**。`free` 的 assert 是调试期捕获脏 ID 的关键，但 release 下空缺（见 S3）。
- `catbase/cat/yaml.cpp::document::load` 修复 mmap 失败时的 NULL → `ryml::strlen(NULL)` UB；影响所有 yaml 配置读取入口（材质/场景描述等），失败场景下从"必崩"变成"返回空 node"，调用方需要能消费空 node（一般已经支持）。

## 整体评价（含 X Critical / Y Warning / Z Suggestion 与结论）

**0 Critical / 5 Warning / 5 Suggestion**

本 commit 是一次目标明确、契约边界清晰的"批量 P0 收口"。每处修复都精准对应实际崩溃路径，且都没有滑入"双保险防御"反模式（release 接 NULL/sentinel 限在 release 类接口；`_loadfile` 用注释契约而非外加 `if`；descAlloc 把 fast-path 修对而不是把 fast-path 直接绕掉）。

需要在下一次小修中跟进的两点是：
1. **W1 BOM**：当前仓库约定 UTF-8 无 BOM，但 shader 文件源自工具链多样，建议加一行 BOM skip，否则该崩在 `_loadfile` 修好后会从"越界 crash"退化成"GLSL 编译错误"，依旧令人困惑。
2. **W3/S3 descAlloc**：把"满"语义封进 `isFull()`、`free` 越界 release 下也守门——本次 commit 已经把最锋利的边修了，再补这两步即可让该子系统在 release 下也不出现内存损坏。

其余 3 条 Warning（W2/W4/W5）属于"靠注释维持的契约"，是仓库一贯的轻防御风格，可接受；建议以 `assert` 形式机器化巩固。Suggestion 全部为加分项，不阻塞 merge。

**结论：可合入。** 建议同 PR 顺手补 W1（BOM）与 S3（descAlloc::free release 守门）。
