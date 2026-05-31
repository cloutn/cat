# Code Review — 04dde69 `objectIDMap` 引入 mutex / atomic_inc（被 249d1e3 回退）

- 范围：`catbase/cat/objectIDMap.h`（新增 `<scl/thread.h>`、`m_mapMutex`、`volatile m_id`，`add/del/get` 加锁，`alloc_id` 改 `scl::atomic_inc`，顺手把 `get/del` 的 `fi == -1` 改成 `-1 == fi`）
- 提交日期：本 commit 后被 249d1e3（“反思过度防御”）整体回退
- 评审角色：5 专家（性能 / 崩溃 / 逻辑 / 规范 / 跨平台编译）
- Commit ↔ Diff 一致性：✗ subject 自述「线程」主题，但 diff 同时夹带 Yoda 风格修改（`-1 == fi`），与 subject 不符；且 commit message 把 **bug 描述当 subject**（“`alloc_id` 非原子 …”），不是“修复什么”而是“什么 bug”。

---

## 修复效果总评

**结论：方向错误，整 commit 应当回退（事实上后续 249d1e3 已经回退）。**

`catbase/cat/objectIDMap.h` 当前文件头部已有显式注释：

```10:11:catbase/cat/objectIDMap.h
// 调用约定：单线程（详见 AGENTS.md §7 — cat / catbase 模块不引入 mutex / atomic / thread）。
// 业务侧 new/delete Object、查表 objectByID 都跑在主线程，未来若引入多线程需先补线程模型再改造。
```

本 commit 的 commit message 自承认 “当前 cat 业务侧 new/delete Object 都在主线程，物理 worker 接入后只读 get”——既然单线程，引入 `scl::mutex` + `scl::atomic_inc` 即 **过度防御**，且违反 AGENTS.md §7 明文约定（cat / catbase 模块不引入 mutex / atomic / thread）。

更糟糕的是：即便假设“未来 physics worker 会读 get”，本 commit 选择的同步方案（atomic_inc + 单一 mutex 保护 hash_table）也 **不能正确支撑** single-writer-many-readers 场景——下文 §三、§五会展开。

---

## 一、性能（Performance）

| 等级 | 项 |
| ---- | ---- |
| 中（M） | `get` 路径加锁，对热路径（场景树渲染每帧 lookup）引入无谓 cost |
| 低（L） | `alloc_id` 用 `atomic_inc` 替换 `++m_id`，单线程下 atomic 比普通自增贵几倍至十几倍（LOCK 前缀 / xadd 约 5–25 ns），但调用频率低，实际不可见 |

### 1.1 `get` 加锁的 cost 测算
- `scl::mutex` 在 Windows 下大概率包 `CRITICAL_SECTION`，在 Linux 下 `pthread_mutex` fast-path（无竞争）≈ 20–30 ns。
- commit message 的“1024 objects 上 = 25 us/frame ≈ 0.15% 60 fps 预算”计算本身没错，但是它隐含了一个 **错误前提**：lookup 调用次数 = 对象数。实际场景树每帧 lookup 次数远不止 1024（picking / event routing / animation rebind / parent-child relink 都会触发 `objectByID`），上限可达 1e4–1e5 量级，扩到 0.5–5% 不再无害。
- **关键反驳**：该 cost 在 cat 单线程模型下是 **零收益的纯支出**。AGENTS.md §7 已明确告知此模块单线程，加锁不带来任何并发正确性收益。

### 1.2 不该出现的可扩展性陷阱
- 即使未来真要走多线程，`get` 路径也应优先考虑 read-mostly 结构（RCU / generation counter / immutable snapshot），而不是无脑给 `hash_table` 套 mutex；后者会让 reader 与 reader 之间也排队，违反 “single-writer-many-readers” 这个 commit 自己声称的目标。

---

## 二、崩溃 / UAF（Crash / UAF）

| 等级 | 项 |
| ---- | ---- |
| 信息 | 本 commit 不直接引入崩溃，但其同步方案 **不正确**，在多线程真正接入后会反过来引入崩溃 |

### 2.1 `atomic_inc` + `mutex` 组合并不构成正确同步
- commit message 的方案 A：`alloc_id` 走 `atomic_inc`，`add/del/get` 走 `m_mapMutex`。
- 这个组合的缺陷是：**`alloc_id` 的递增与 `add` 的写表不在同一把锁内**。多线程场景下序列：
  1. 线程 T1：`alloc_id()` → 拿到 id=42；
  2. T1 还没来得及 `add(obj_with_id_42)`；
  3. 线程 T2：`get(42)` → 进入 `m_mapMutex`，查表 → 找不到 → 返回 NULL；
  4. T2 把 NULL 当成“对象不存在”做后续处理（缓存清理、hash 失效等），其实对象正在 in-flight 创建。
- 业务上是否能容忍？取决于上层。但 commit 完全没解释 alloc/add 之间的窗口语义；即“发了 id 但还没入表”这段时间内 `get(id)` 应该返回什么。这是 **方案设计上的 hole**，不是单纯加锁就能填的。
- 同理 `del` 与 `get`：T1 `del` 后 hash_table 已 erase，T2 仍持有该 id 调用 `get` → 返回 NULL，之前在缓存里持有的 `T*` 是否还安全？没人定义。

### 2.2 hash_table 扩容期的 read 安全
- `scl::hash_table` 的 `add` 在容量不足时会 rehash。`get` 与 `add` 用同一 mutex 互斥后扩容这一刻是安全的，但前提是 `get` 没漏锁。当前 diff `get` 第一行就 `mutex_lock lock(&m_mapMutex);`，看起来挡住了，但与 §三的 init 早返回顺序有冲突（见下）。

---

## 三、逻辑（Logic）—— 本 commit 的核心硬伤

| 等级 | 项 |
| ---- | ---- |
| 高（H） | `alloc_id` 溢出后 **无 saturate**，下一次调用仍然 `INT_MIN+1 ≤ 0` 返回 -1，但 `m_id` 已被永久污染为负数；并发下还能被多次 wrap 到任意负值，sentinel `-1` 失效 |
| 高（H） | `get` 内 `mutex_lock` 在 `m_map.is_init()` 检查 **之前**——但同样的，`add/del` 的 `is_init()` 检查在 `assert` 里也在锁后，整体顺序不一致也不重要，但暴露设计未经推敲 |
| 中（M） | `volatile int m_id` 与 `scl::atomic_inc` 双重声明属于 **类型噪音**，混淆读者：到底是要靠 volatile 还是靠 atomic 提供可见性？ |

### 3.1 `alloc_id` 溢出语义被打破
原版（保留在 HEAD）：

```73:83:catbase/cat/objectIDMap.h
template <typename T>
int ObjectIDMap<T>::alloc_id()
{
	// 不要写 m_id + 1 > 0：signed int 溢出是 UB，编译器有权把它优化掉。
	// release 也必须能拦下，避免回绕成负数后污染 -1 这种 sentinel。
	if (m_id >= INT_MAX - 1)
	{
		assert(false);
		return -1;
	}
	return ++m_id;
}
```

这版的关键性质：**饱和**——一旦达到上限就永远拒绝再发 id，`m_id` 不会越过 INT_MAX-1。

本 commit 的新版：
```cpp
const int newId = scl::atomic_inc(&m_id);
if (newId <= 0) { assert(false); return -1; }
return newId;
```
- `atomic_inc` 在 INT_MAX 上 +1 在 C/C++ 层面对 signed int 仍是 UB；x86 实测 wrap 到 INT_MIN，但这是“platform-defined 巧合”，不是 spec 保证。
- 即使运行时 wrap 成功：`newId == INT_MIN`，函数返回 -1 拒绝。**但 `m_id` 已经被原子性地写成了 INT_MIN**。下次 `alloc_id` 拿到 `INT_MIN+1 = -2147483647`，仍 ≤ 0，又拒；以此类推，每次都把 `m_id` 推进一步，直到再 wrap 回 0…然后开始发 1, 2, 3——**与原本被销毁却仍在表中的 id 冲突**，sentinel 直接失效。
- 多线程进一步劣化：N 个线程同时在 INT_MAX 边界 `atomic_inc`，各拿到 INT_MIN+0..N-1，全部 ≤ 0 返回 -1，但 `m_id` 已被推到 INT_MIN+N。
- **修复版即便要保留 atomic，也必须 saturate**：CAS 循环 `if (cur >= INT_MAX-1) return -1; CAS(cur, cur+1)`。本 commit 没做。

### 3.2 `volatile int m_id` 是噪音
- `scl::atomic_inc` 内部一定是 LOCK XADD / interlocked intrinsic / std::atomic_ref，**已经自带 acquire/release**。`volatile` 不提供任何额外保证。
- 而对其它读者（比如未来 worker 直接读 `m_id` 而非走 `alloc_id`），`volatile` 在 GCC/Clang 上不构成跨线程同步——这是 §五跨平台维度问题。
- 加 `volatile` 反而误导读者“这里靠 volatile 保证可见性”，掩盖真正的同步点。

### 3.3 `get` 锁与 `is_init` 顺序
- diff 后：
  ```cpp
  scl::mutex_lock lock(&m_mapMutex);
  if (!m_map.is_init()) return NULL;
  ```
- 这里逻辑没崩，但 **第一次调用 `get` 时，`m_map` 未 init**，仍要付一次 mutex 进出 cost。这进一步说明加锁是无谓 overhead。

---

## 四、规范（Conventions）

| 等级 | 项 |
| ---- | ---- |
| 严重（Critical） | 直接违反 AGENTS.md §7：cat / catbase 模块不引入 mutex / atomic / thread。该约定有明文，本 commit 自承认单线程仍引入 `scl::mutex` + `scl::atomic_inc` |
| 高（H） | commit message 把 bug 描述写成 subject（“`alloc_id` 非原子 …”），违反 cat 仓库 commit style——subject 要写“修复什么”，bug 描述放 body |
| 高（H） | 夹带：把 `get/del` 的 `fi == -1` 顺手改成 Yoda `-1 == fi`，与 subject 主题（线程）无关，应单独 commit 或与现有 Yoda 化提交一起做 |
| 中（M） | 与紧随的 249d1e3 全量回退构成 “加 → 退” 折返，浪费 review 与 git history。明知模块单线程，仍走一次完整加锁尝试再退回，规范瑕疵 |
| 信息 | 缩进/Allman/Tab/`m_` 前缀/`NULL`/`scl::`：本 diff 内符合 |

### 4.1 AGENTS.md §7 违反度
- §7 是模块边界硬性规则，不是建议。文件头注释（HEAD 第 10–11 行）也复述了一遍。本 commit 等于同时违反 AGENTS.md 与文件本地注释。
- 如果有合理的多线程需求（physics worker），正确流程是：先在 doc 里补完整线程模型说明（哪些字段被哪些线程读写、生命周期归属、退出时序），评审通过后再动 cat/catbase。本 commit 跳过该流程。

### 4.2 commit subject 反模式
- 反例：`[线程] cat/objectIDMap.h:65-69 — alloc_id 非原子...`
  - 含具体行号，与 IDE 状态绑定，下次别人改完就漂移；
  - 描述的是 bug，不是 fix；
  - 不答“为什么改”。
- 应写：`[catbase] objectIDMap：补 alloc_id 的并发可见性（待评审线程模型）`，bug 细节 + 触发条件放 body。

### 4.3 夹带
- 风格修改（Yoda 化）应当属于一次集中的“风格统一”commit，不能塞进“线程化”commit。
- 如果 reviewer 想 revert 线程化部分，会连带把 Yoda 化也退掉，破坏风格一致性；事实上 249d1e3 整体回退后，那两处 Yoda 改动也一起没了。

### 4.4 折返 (加 → 退)
- 04dde69 加锁，249d1e3 立刻回退——意味着这次提交对仓库无净贡献，但永久留在 git history 里。
- 改进流程：在 cat/catbase 这种有硬约束的模块下手前，先在 PR/讨论里 sanity-check（“§7 规定 …，这次想 …，是否需要先升级 §7”），避免无效改动。

---

## 五、跨平台编译（Cross-platform Compile）

| 等级 | 项 |
| ---- | ---- |
| 警告（W） | `volatile int` 的内存语义在 MSVC（/volatile:ms）与 GCC/Clang（ISO volatile）上语义不同 |
| 警告（W） | `scl::atomic_inc` 对 `int*`（非 `volatile int*` / 非 `std::atomic<int>*`）的输入在 ISO C++ 下即便在 x86 实现 OK，到 ARM 等 weak-memory ISA 上的 barrier 由 scl 内部实现决定，未审视 |
| 信息 | 单 mutex 模型在所有平台都能编译通过，不是编译期问题 |

### 5.1 `volatile` 在 MSVC vs GCC/Clang
- MSVC 默认 `/volatile:ms`：volatile load/store 等价于 acquire/release（MS 历史扩展）。
- GCC/Clang（含 MSVC `/volatile:iso`）：volatile **只防编译器优化重排，不防 CPU 重排，更不构成跨线程同步**。
- 本 diff 既然引入了 `atomic_inc`，就应当 **删除** `volatile`：要么靠 atomic 完整提供同步，要么改成 `std::atomic<int>` / `scl::atomic<int>` 这种类型自描述的容器。当前“volatile + atomic_inc”混用的写法会让 reader 在不同平台上得出不同结论。

### 5.2 `scl::atomic_inc` 接口签名
- 看不到 scl 实现就无法判定其是否对 int* 做 std::atomic_ref<int> 包装，还是直接 `_InterlockedIncrement` / `__sync_add_and_fetch` / `__atomic_add_fetch`。
- 在 ARM64 / weak memory model 下，`atomic_inc` 必须保证 acq_rel 或 seq_cst 才能让 `add()` 写表与 `alloc_id()` 自增之间形成 happens-before。本 diff 没看 scl 实现就直接用，依赖隐含承诺。
- 在 cat 单线程模型下这些都不成问题——但这恰恰反过来说明本 commit 的引入是无收益负担。

### 5.3 include 边界
- `+#include <scl/thread.h>`：catbase 引 scl 的线程头不构成模块边界违规（scl 是底层共享层），但与 §7 “catbase 不引入 thread” 在精神上冲突——文件可见 `<scl/thread.h>` include 之后，团队其他人会以为 catbase 已经准备好走多线程，引发 cargo-cult 扩散。

---

## 修复建议（按优先级）

1. **(Critical) 整 commit 回退**——后续 249d1e3 已经做了。事后补救：在 `objectIDMap.h` 文件头注释里增加“2026-05 曾尝试加 mutex/atomic_inc 后回退，未来若需要并发请先升级 AGENTS.md §7”，作为 history 标记，避免后人重复尝试。
2. **(Critical) 治理流程**：cat / catbase 模块的 commit 在 CI 或 pre-commit hook 增加 `git diff` 中检测 `scl::mutex|scl::atomic|<scl/thread`，命中即 fail，引用 AGENTS.md §7。
3. **(H) commit message 风格**：补一条 cat 仓库 commit guideline，明确 subject 写 “修复什么 / 改了什么”，禁止把行号或 bug 描述放 subject。
4. **(H) 夹带管理**：纯风格修改（Yoda 化）应单独成 commit；reviewer 看到夹带应直接打回。
5. **(M) `alloc_id` saturate 语义**：如果未来真要走 atomic，写 CAS 循环以保留原版的 saturate 性质，不能放任 `m_id` 在溢出后被推成 INT_MIN 持续累加。
6. **(M) `volatile` 与 `atomic_*` 二选一**：保留任何一个就够；混用会跨平台行为分歧。
7. **(L) get/add/del 的 single-writer-many-readers 设计**：若未来 physics worker 真接入，正确路径是 immutable snapshot / RCU / 双 buffer，而不是单一 mutex 给 hash_table 套个全局排队。设计阶段就要把 alloc/add 之间“已发 id 未入表”的窗口语义定义清楚。

---

## 一句话结论

> **本 commit 在明文单线程的模块上引入 mutex + atomic 是规范级（Critical）违反；同步方案本身（atomic_inc 不饱和 + mutex 不覆盖 alloc/add 窗口 + volatile 跨平台语义分歧）也不正确；commit message 把 bug 当 subject、夹带 Yoda 风格修改；249d1e3 全量回退是正确处置，但折返本身已是规范瑕疵——治本应在 CI 加边界检查防止此类提交进入仓库。**
