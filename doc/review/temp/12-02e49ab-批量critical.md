# 02e49ab 批量 Critical 审查报告

> 五专家视角：性能 / 崩溃 / 逻辑 / 规范 / 跨平台编译  
> 范围：`cat/gltfLoader.cpp`、`cat/material.cpp`、`cat/object.h`、`cat/scene.h`、`cat/skin.cpp`、`cat/skin.h`、`catbase/cat/objectIDMap.h`

---

## 一、Commit 一致性

**【Critical · 规范】** commit subject 写成了「review 报告片段列表」（多行 bullet 形式的 bug 描述），不是简明的"做了什么"。

- cat 风格隐含约定：subject 一行说清意图（参考 `3df3ecb`、`249d1e3` 等历史 commit）。
- 后果：`git log --oneline` 不可读；`git blame` 定位时无法快速判断意图；后续 `249d1e3` 反思里也将本 commit 标记为"过度防御浪潮"高峰（参考 `doc/review/代码审查-反思-过度防御与冗余修改.md` §6）。
- 建议拆 commit：
  1. `[fix] Material 未初始化 m_env`（真 bug）
  2. `[fix] ObjectIDMap::alloc_id signed 溢出 UB / init 乘法溢出`（真 bug）
  3. `[refactor] gltfLoader 加 size_t→int 截断防御`（边界硬化）
  4. ⚠️ `[防御] Object::child / Scene::object 越界返回 NULL` —— 见下文 Critical，**不建议合入**

---

## 二、Critical

### C1【规范】commit subject 是 bug 列表

见上节。是用户已点名的核心 critical。

### C2【逻辑】`Object::child(int)` / `Scene::object(int)` 从契约模式改宽容模式 —— 上游 caller 全不检 NULL

**改动**：

```cpp
Object* child(int index)
{
	return (index >= 0 && index < m_childs.size()) ? m_childs[index] : NULL;
}
```

**调用方扫描**（`->child(` / `.child(int` 实际 caller）：

| 位置                            | 上下文                                          | 是否检 NULL |
| ----------------------------- | -------------------------------------------- | ------- |
| `cat/scene.cpp:62`            | `for (i < object->childCount()) queue.push(object->child(i))` | 否       |
| `testCat/mainGUI.cpp:323`     | `_onGUIObject(object->child(i), ...)` 在 for 循环里 | 否       |
| `testCat/testPrimitive.cpp:73,84` | `Object* obj = root->child(i);` 在 for 循环里 | 否       |
| `testCat/client.cpp:505`      | `m_scenes[sceneIndex]->object(objectIndex)` return | 否       |

**问题**：

- 所有 caller 本就在 `for (i < childCount())` 里，契约满足，原 `return m_childs[index]` 完全正确。
- 改宽容后 → `queue.push(NULL)` / `_onGUIObject(NULL)` / `Mesh::draw(NULL)` 把"立即越界崩"变成"NULL 晚崩"，**信噪更差**。
- 与反思文档 #11「不变量靠 assert 暴露违反」、cat 风格 §1.5 fail-loud 原则**直接冲突**。
- `09-249d1e3-反思过度防御.md` 已明确把该改动列为反思对象（"唯一外部 caller 在 `for(i<childCount())` 里，契约满足"），后续 `249d1e3` 已删除。

**建议**：本改动应在本 commit 直接砍掉，保留 `return m_childs[index]`，由 `index < size()` 的契约 + `scl::vector::operator[]` 的 assert 兜底。

---

## 三、Warning

### W1【性能 / 设计】`Skin::m_jointMatricesCapacity` 字段冗余

- 同 commit 里 `setInverseBindMatrices` 已 `safe_delete_array(m_jointMatrices)`；下次 `generateJointMatrix` 走 `NULL == m_jointMatrices` 自然重建。
- 容量字段**唯一覆盖**的场景是「joints 增长但 IBM 不变」—— 但 `Skin::addJoint` 自身才是该不变量的边界，应在 `addJoint` 里 `safe_delete_array(m_jointMatrices)`（与 `setInverseBindMatrices` 对齐），而非引入"容量字段 + 比较扩容"的状态机。
- 增加 1 个成员变量 + 3 处维护点（ctor / dtor / setIBM / generate），**净复杂度 > 净收益**。
- 反思文档 #6 已劝阻并在 `249d1e3` 删除，且改 `addJoint` 同步 `safe_delete_array`。
- 性能：装配阶段 N 次 addJoint 触发 N 次 `delete[]`/`new[]`，但 N=几十~上百，不在 hot path（hot 是每帧 `generateJointMatrix`），**实际成本可忽略**。

### W2【崩溃 / 设计】`Material::release()` 重置 `m_render` / `m_env` 不必要

```cpp
void Material::release()
{
	if (NULL != m_env && NULL != m_textureFile)
		m_env->releaseTextureFile(m_textureFile);
	m_textureFile	= NULL;
	m_env			= NULL;   // ← 多余
	m_render		= NULL;   // ← 多余
}
```

- `Material::init(IRender*, const char*, Env*)` 接受三参数 → 即便 release 后再 init，也会重新覆盖 `m_render` / `m_env`，**置 NULL 没有可观察的语义价值**。
- 单纯加了"release 后到下一次 init 之间不能调用任何依赖 m_render / m_env 的方法"的隐式契约，但 `texture()` / `init()` 之外没有这种依赖。
- 双 NULL 守卫 `m_env != NULL && m_textureFile != NULL` 本身合理（防 init 失败 / 未 init 释放），保留 OK。
- 建议：只保留 `m_textureFile = NULL`；`m_env` / `m_render` 不动（与 ctor 状态保持镜像，仅 dtor 不需关心）。

### W3【规范 / 可读性】`_flattenVertexAttrs` / `_loadAnimChannel` 三处 NULL 边界检查叠加

```cpp
if (data->attributes_count > static_cast<size_t>(INT_MAX) ||
    NULL == data->attributes[0].data ||
    data->attributes[0].data->count > static_cast<size_t>(INT_MAX))
```

- `attributes_count > INT_MAX` 在真实 gltf 里基本不可能（21 亿个 attribute）；该分支属于"理论可能 / 实际不存在"。
- `_loadAnimChannel` 同理：`timeAccessor->count > INT_MAX` 不会发生（21 亿关键帧）。
- 但既然 cgltf API 给出 `cgltf_size`（=`size_t`），从严格 32-bit 安全角度保留也无害；**建议合并到一行 `assert(false)` 早退**，避免在 hot path 散开三个分支。
- 真正有价值的是：`NULL == sampler->input || NULL == sampler->output` 与 `timeAccessor->count != frameAccessor->count`（cgltf 部分失败时确实可能命中）。

### W4【性能】每帧 `Object::child` / `Scene::object` 多 1 次范围检查

- 改宽容后每次访问多 1 次 `index >= 0 && index < size()` 比较。`scene.cpp:62` 在场景遍历队列里，最坏 O(N) 每帧。
- N 个 Object × 每帧 → 可观察成本，但相对于 GPU 工作量可忽略。
- 真正的损失是 C2 描述的语义损失，不是这点 CPU。

---

## 四、Suggestion

### S1 `gltfLoader::_loadAnimChannel` frameCount 比较删除 —— 正确

- 删除 `frameCount != static_cast<int>(frameAccessor->count)`，因为 `timeAccessor->count != frameAccessor->count` 已等价覆盖。逻辑等价，无副作用。✓

### S2 `ObjectIDMap::alloc_id` 修 signed 溢出 UB —— 正确

```cpp
// before: assert(m_id + 1 > 0);   // signed overflow UB, 编译器可优化掉
// after:  if (m_id >= INT_MAX - 1) { assert(false); return -1; }
```

- 配合 `add()` 入口处 `if (obj->id() < 0) return;` 拦下 -1 sentinel 污染表，链路完整。✓

### S3 `ObjectIDMap::init` 乘法溢出反向写 —— 正确

```cpp
if (maxCount <= 0 || maxCount > INT_MAX / MAX_CONFLICT) { assert(false); return; }
```

- 反向写避免乘法本身 UB，方向正确。✓
- 加 `maxCount <= 0` 入口防御，配 `MAX_CONFLICT * maxCount = 0` 的退化也挡住，OK。

### S4 `Material::Material()` 补 m_env(NULL) —— 真 bug 修复

ctor 漏初始化是真 bug，本次修法正确。release 双 NULL 守卫保留即可（见 W2）。

### S5 `#include <limits.h>` 引入 INT_MAX

跨平台 OK，C 风格 header（cat 风格容许）；优先级低。

---

## 五、影响范围

| 文件                              | 改动性质                  | 风险             |
| ------------------------------- | --------------------- | -------------- |
| `cat/gltfLoader.cpp`            | size_t/int 边界 + NULL 守卫 | 低（fail-loud 早退） |
| `cat/material.cpp`              | ctor + release 修正     | 低 ~ 中（release 重置 m_render/m_env 见 W2） |
| `cat/object.h`                  | child(int) 改宽容        | **中** —— 4 处 caller 全不检 NULL（C2） |
| `cat/scene.h`                   | object(int) 改宽容       | **中** —— 同上    |
| `cat/skin.cpp` / `cat/skin.h`   | 加 m_jointMatricesCapacity 字段 | 低（已被 249d1e3 删除） |
| `catbase/cat/objectIDMap.h`     | 溢出守护                  | 低（真 bug 修复）    |

调用方扩散点：`scene.cpp:62`、`testCat/mainGUI.cpp:323`、`testCat/testPrimitive.cpp:73/84`、`testCat/client.cpp:505`。

---

## 六、整体评价

本 commit 是**真 bug 修复 + 防御性扩张**的混合体，混合得有点过。

**值得保留**（高质量）：

- `Material::Material()` 补 `m_env(NULL)` —— 真 bug
- `ObjectIDMap::alloc_id` 修 signed 溢出 UB —— 真 bug
- `ObjectIDMap::init` 乘法溢出反向写 —— 真 bug
- `gltfLoader._loadAnimChannel` `sampler->input/output` NULL 与 count 不一致 —— cgltf 部分失败的真实路径
- `gltfLoader._loadAnimChannel` 删除冗余 frameCount 比较 —— 逻辑等价清理

**应当删掉**（过度防御 / 反契约）：

- `Object::child(int)` / `Scene::object(int)` 改宽容模式 —— **Critical**，与 cat 风格 §1.5 fail-loud + 反思 #11 直接冲突，所有 caller 不检 NULL
- `Skin::m_jointMatricesCapacity` 字段 —— Warning，反思 #6 已删除，应改 `addJoint` 同步 reset
- `Material::release()` 把 `m_render` / `m_env` 也置 NULL —— Warning，无可观察语义

**核心问题**：

1. **commit subject 写成 review 片段**（Critical · 规范）
2. **批量改契约式访问器为宽容模式**（Critical · 逻辑）—— 与 cat 风格"简洁 + fail-loud"直接矛盾，是反思文档 §6 / §11 的核心反面教材
3. AGENTS.md §1.4「保持可读性追求简洁」被违反 —— 多处分支叠加 + 状态字段，净复杂度增加而 bug 仅在理论分支

**建议处置**：拆 4 个 commit，C2 / W1 部分不入主线（已被 `249d1e3` 反向回退验证此判断）。
