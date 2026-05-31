# Code Review · 3df3ecb · Skin 装配修复（IBM/joint NULL 兜底）

> 审查对象：`cat/gltfLoader.cpp` + `cat/skin.cpp`
> 审查角度：性能 / 崩溃安全 / 逻辑正确 / 引擎规范 / 跨平台编译
> 结论：**整体方向正确，建议合入；有 2 处必改、3 处建议改、若干风格问题**

---

## 0. TL;DR

| 维度 | 等级 | 说明 |
|------|------|------|
| 崩溃安全 | A- | NULL/空 IBM/空 joint 三条崩溃链全部封堵 |
| 逻辑正确 | B+ | `m_jointMatrices` 分配大小用旧值偏大，逻辑安全但歧义；`count<=0` 早退顺序 OK |
| 性能 | A | 无热路径回归；唯一新增是 `min` 比较和一次空指针判断 |
| 规范 | B | NULL/Yoda/safe_delete 基本到位；`size_t` vs `int` 与纵向对齐有瑕疵 |
| 跨平台编译 | A | 无平台 API；`static_cast<int>` 显式收敛，无窄化告警风险 |

---

## 1. 必改（Must Fix）

### 1.1 `m_jointMatrices` 分配大小应改为 `count`，不要用 `m_inverseBindMatrixCount`

**位置**：`skin.cpp generateJointMatrix`

```cpp
if (NULL == m_jointMatrices)
    m_jointMatrices = new matrix[m_inverseBindMatrixCount];   // ← 用旧字段

for (int i = 0; i < count; ++i) { ... }                       // ← 用 min
```

**问题**：

- 当前虽然**不会越界**（`count = min(jointCount, IBMCount) <= IBMCount`，分配 `>=` 使用），但：
  - 读者必须二次推理才能确认安全 —— 违反 cat 风格里"代码自解释"的隐含约定
  - `matrixCount = count` 返回给调用方，`m_jointMatrices` 物理长度却是 `IBMCount`，**长度语义不一致**
  - 若后续有人加"`memset(m_jointMatrices, 0, sizeof(matrix)*matrixCount)`"或类似按返回值遍历的操作，没问题；但若按 `m_inverseBindMatrixCount` 遍历就会读到未初始化的尾部矩阵（`i ∈ [count, IBMCount)`）
- 你在重点关注里也提到"建议改成 `new matrix[count]` 更清晰"——此项确认必改

**修法**：

```cpp
if (NULL == m_jointMatrices)
    m_jointMatrices = new matrix[count];
```

> 注：commit message 提到"下一个 commit 02e49ab 加 `m_jointMatricesCapacity`" —— 那次重构是为了 hot-reload，与本条无冲突；本条只关心"分配长度 = 使用长度"的清晰度。

### 1.2 `_loadSkin` 路径里 `safe_delete_array(ibm)` 漏在 NULL 分支外释放

**位置**：`gltfLoader.cpp _loadSkin`

```cpp
matrix* ibm = _loadIBM(...);
if (NULL == ibm)
{
    log_warning(...);
    outSkin->setInverseBindMatrices(NULL, 0);
}
else
{
    outSkin->setInverseBindMatrices(ibm, jointCount);
    safe_delete_array(ibm);     // ← 仅在 else 分支
}
```

这里 NULL 分支没 `safe_delete_array` 是**正确**的（ibm 本就是 NULL，`safe_delete_array` 调它也无害但冗余）。**这条不算问题，撤回。**

⇒ **1.2 撤销，仅保留 1.1 一项必改。**

---

## 2. 建议改（Should Fix）

### 2.1 `setInverseBindMatrices` 中 `m_inverseBindMatrixCount` 的赋值时序歧义

```cpp
safe_delete_array(m_inverseBindMatrices);
safe_delete_array(m_jointMatrices);

if (NULL == matrices || count <= 0)
{
    m_inverseBindMatrices    = NULL;
    m_inverseBindMatrixCount = 0;
    return;
}
m_inverseBindMatrixCount = count;
m_inverseBindMatrices    = new matrix[count];
```

**优点**：异常分支显式置 0，旧 `count = count` 那行被移除，改成成功路径才赋值，避免"matrices 为 NULL 但 count 还残留旧值"的坑。

**建议**：把 `m_inverseBindMatrixCount = 0` **提到 `safe_delete_array` 之后、if 之前**，做无条件先置 0 再按需写入：

```cpp
safe_delete_array(m_inverseBindMatrices);
safe_delete_array(m_jointMatrices);
m_inverseBindMatrices    = NULL;
m_inverseBindMatrixCount = 0;

if (NULL == matrices || count <= 0) return;

m_inverseBindMatrices    = new matrix[count];
m_inverseBindMatrixCount = count;
// memcpy 或循环拷贝（看原代码）
```

理由：**指针/计数同生同灭**，只有一处赋值点，类成员状态机更简单。

### 2.2 `generateJointMatrix` 出参先置 0 的位置

```cpp
matrix* generateJointMatrix(..., int& matrixCount)
{
    matrixCount = 0;                                          // ← 好习惯
    if (NULL == m_inverseBindMatrices || ...) return NULL;
    ...
}
```

**赞**：`matrixCount = 0` 提前到所有 return NULL 之前，调用方拿到 NULL 时 `matrixCount` 也是 0，配对干净。**保持现状即可**，仅备注：这是该提交的亮点之一。

### 2.3 `root()` 里循环类型混用 `size_t` 与 `int`

```cpp
for (size_t i = 0; i < m_joints.size(); ++i) { ... }          // size_t
const int MAX_DEPTH = static_cast<int>(m_joints.size()) + 1;  // int
int depth = 0;
```

cat 引擎里 `scl` 容器 `size()` 返回什么类型需确认。若返回 `size_t`：
- 上面循环用 `size_t` 没毛病
- 但项目里大量用 `int i = 0; i < count` 风格（见 `_loadSkin` 那个 `for (int i = 0; i < jointCount; ++i)`）
- **建议统一成 `int`**，与 `MAX_DEPTH/depth` 对齐：

```cpp
Object*   current      = NULL;
const int jointCount   = static_cast<int>(m_joints.size());
for (int i = 0; i < jointCount; ++i)
{
    if (NULL != m_joints[i]) { current = m_joints[i]; break; }
}
```

---

## 3. 性能（Performance）

- **零热路径影响**：`generateJointMatrix` 每帧每蒙皮一次，新增的早退 + `min` + 一次 NULL 比较，O(1)，可忽略。
- **NULL joint 兜底 `IBM * inverseMesh`** 等价 bindpose：**避免在每帧渲染主循环里跳过这一项导致脏数据**，比直接 `continue` 留旧矩阵安全。
  - 取舍提示：commit-msg 里也写了 249d1e3 后续改成"NULL → return NULL 不兜底"。两种都对：
    - 当前：**单关节缺失 ⇒ 该关节静止于 bindpose**，其余动画照常 → 表现是"半截不动的角色"
    - 后续：**只要有一个 NULL ⇒ 整个 skin 不更新** → 表现是"整体定格"
  - 我个人倾向后续做法（数据完整性比"演示能跑"更值钱），但**本 commit 修法不算错**。
- `_loadSkin` 改 `assert → log_warning + continue` 在加载期，影响为 0。

---

## 4. 崩溃安全（Crash Safety）

逐条核对你列出的崩溃链：

| 旧崩溃点 | 新防御 | 评价 |
|---------|--------|------|
| `setInverseBindMatrices(NULL, n>0)` 后续 `new matrix[n]` 复制 NULL | `count<=0` 早退 + 显式 `m_inverseBindMatrixCount=0` | ✓ |
| `generateJointMatrix` 在 IBM 为 NULL 时 deref | 函数首部三条 NULL/空判断 | ✓ |
| `m_joints[0]` 是 NULL → `root()` deref | 找首个非 NULL joint 作 `current`；fallback 也用 `current` | ✓ |
| `m_joints[i]->globalMatrix()` 当 `m_joints[i]==NULL` | NULL 检查 + IBM*inverseMesh 兜底 | ✓ |
| `_objectByNode` 返回 NULL 后 `assert` 释放版直接崩 | 改 `log_warning + continue`，不再 push NULL | ✓ |
| jointCount > IBMCount 越界写 `m_jointMatrices` | `min(jointCount, IBMCount)` | ✓（前提同 §1.1） |

**新增风险检查**：`addJoint(obj)` 内部假设 `obj != NULL` 吗？已经在调用处 `continue`，OK；但 `m_joints` 里**仍可能因为 setInverseBindMatrices 之外的路径被塞 NULL** —— 本 commit 在 `_loadSkin` 已挡住，但 `addJoint` 自身**建议加 `if (NULL == joint) return;` 防御**（不在本 commit 范围，可下个 commit 处理）。

---

## 5. 引擎规范（cat 风格）

| 项 | 状态 |
|----|------|
| `NULL` 而非 `nullptr` | ✓ 全部使用 NULL |
| Yoda 比较 | ✓ `NULL == ibm` / `NULL == m_root` 已修；`depth >= MAX_DEPTH` 是关系比较，无需 Yoda |
| `safe_delete_array` | ✓ 新增 `m_jointMatrices` 同步释放 |
| `scl::log` include | ✓ `gltfLoader.cpp` 顶部新增 `#include "scl/log.h"` |
| 单线程假设 | ✓ 无锁、无原子，符合 |
| `m_/s_` 前缀 | ✓ `m_jointMatrices` 等 |
| 纵向对齐 | △ `m_inverseBindMatrices    = NULL;` 与 `m_inverseBindMatrixCount = 0;` 在 if 块里有对齐，赞；但 `Object*\tcurrent = NULL;` 单独一行没体现对齐压力，OK |
| Tab + Allman | ✓ diff 里大括号换行符合 |
| 末尾空行 | ✓ 未触碰 |
| UTF-8 无 BOM | 假设 IDE 已配置；diff 不可见，**请确认 `gltfLoader.cpp` 顶部新增 include 行未引入 BOM** |

**小瑕疵**：

```cpp
if (NULL != m_joints[i]) { current = m_joints[i]; break; }
```

cat 风格通常 Allman，单行 if 块**建议拆开**：

```cpp
if (NULL != m_joints[i])
{
    current = m_joints[i];
    break;
}
```

---

## 6. 跨平台编译

- 无平台 API，无 `__declspec` / `__attribute__`。
- `static_cast<int>(m_joints.size())` 显式收敛，避免 MSVC C4267（`size_t -> int`）告警。✓
- `log_warning` 使用 `printf` 风格 `%d` 占位符，`int i` 类型匹配。✓
- 没有 `auto`、没有 C++17/20 特性，**符合 cat 老 C++ 标准**。
- Vulkan/Metal/GL 后端无关，纯 CPU 数据路径。

**风险**：无。

---

## 7. 综合建议

**合入策略**：合入前修 §1.1 一处（`new matrix[count]`），其余可作为后续清理。
**后续 commit 建议**（与你提到的 02e49ab 对齐）：
1. `m_jointMatricesCapacity` 解决 hot-reload IBM 变更时未重分配的问题
2. `addJoint(NULL)` 自身防御
3. `root()` 循环 `size_t/int` 风格统一

**亮点**：
- `setInverseBindMatrices` 同步丢弃 `m_jointMatrices` 缓存，**避免 IBM 变更后用旧 capacity 的隐性 bug** —— 这个细节做对了，赞
- `matrixCount = 0` 出参提前置零，配 NULL 返回，**调用方写法可以不必 if (ret) 判断 matrixCount**

---

**审查人**：5-expert panel (perf/crash/logic/style/portability)
**结论**：**Approve with one nit**（§1.1）
