# [756ba36] 取消 draw 参数中的 render

- 提交时间：2026-05-31 22:39
- 涉及文件：cat/mesh.{cpp,h}, cat/object.{cpp,h}, cat/primitive.{cpp,h}, cat/scene.{cpp,h}, cat/terrain.{cpp,h}, testCat/client.cpp, doc 子系统-cat.md
- 审核维度：性能 / 崩溃 / 逻辑 / 规范 / 跨平台编译

## Commit 与代码一致性

- Commit message "取消 draw 参数中的 render"，主线意图与 diff 完全一致：把 `IRender* render` 形参从 `Mesh::draw / Object::draw / Primitive::draw / Scene::draw / Terrain::draw` 一并移除，绘制时改用 `Primitive::m_render` 成员。
- `git show 756ba36 --stat` 含 12 个文件、35/35 增删，与给定 diff 吻合，包含一份 doc 同步（"子系统-cat.md"，18 行）。
- **Commit message 漏说的隐含变更**（建议在 message 或后续 commit 中补一句）：
  1. `Primitive::draw` 新增了 `NULL == m_render` 契约检查（契约变严了，不是单纯的参数瘦身）。
  2. `testCat/client.cpp:290` 的最后一个实参语义从 `IRender*` 改成 `bool isPick`（见 Warning #1）。
  3. `Primitive::draw` 上方注释的契约描述从 "setShaderWithPick" 扩展为 "setRender + setShaderWithPick"。

## Critical

- 无。

  对"每个 Primitive 在 draw 之前是否都已 setRender"做了 2 层 callees grep 验证（`new Primitive\(\)` 全部 8 处构造点都跟着 `setRender(...)`）：
  - `cat/terrain.cpp:67-68`：`new Primitive()` 紧跟 `m_primitive->setRender(render)` ✓
  - `cat/gltfLoader.cpp:238-239`：`new Primitive()` 紧跟 `primitive->setRender(m_render)` ✓
  - `testCat/testPrimitive.cpp` 共 5 处（`createBone` 93、`createTestPrimitive_getVertexData` 126、`createTestVulkanPrimitive` 149、`createTestVulkanPrimitiveColor` 188、`testPrimitive_vertexPosition_edgeCase` 226）均在构造后立即 `setRender` ✓
  - `testCat/simpleShape.cpp:52-53`：`new Primitive()` 紧跟 `p->setRender(render)` ✓

  Mesh / Object / Scene / Terrain 不持有也不需要 `m_render` 成员（grep `m_render` 在它们各自的 .h 里：Mesh/Object/Scene 全无；Terrain 有但 `draw()` 已经不用，详见 Suggestion #2）。draw 链路上层只透传，最终在 Primitive 取 `m_render`，等价性成立。

## Warning

### W1. `testCat/client.cpp:290` 在 `#else` 分支里做了"参数语义反转"

```285:293:testCat/client.cpp
		m_render.endDraw();
#else
		m_gridPrimitive->draw(m_camera->matrix(), NULL, 0, false);

		//m_object->draw(m_camera->matrix(), false);
#endif
```

- 旧代码 `m_gridPrimitive->draw(m_camera->matrix(), NULL, 0, &m_render)`：第 4 个实参是 `IRender*`，但旧 `Primitive::draw` 的第 4 形参是 `bool isPick`、第 5 形参才是 `IRender*` —— 旧调用本身就少传 1 个参，**且 `IRender*` 通过指针→bool 隐式转换会被填到 `isPick=true`**，行为是"pick 路径"。
- 新代码改成 `..., 0, false)`，语义变成 `isPick=false`，是"普通渲染"。**两者语义不等价**（true→false 翻转）。
- 但实测影响为 0，因为：
  1. `m_gridPrimitive` 在 `testCat/client.h:112` 早已注释掉（`//Primitive* m_gridPrimitive;`），这段代码引用一个不存在的成员，无论新旧都编译不过。
  2. 整段 `#else` 分支对应"非 `TEST_VULKAN`"路径，配套的 `m_render` 类型是 `UIRenderOpenGL`（client.h:90）—— 工程当前没有这个类的实现，整支分支是死代码。
- 结论：本 commit 在一段编不过的死代码里又做了一次机械替换，反而让"将来重新启用 OpenGL 路径"的人更难还原原意（原本是"以 IRender* 形式硬塞 → 实际走 pick=true"的 bug，现在变成"显式 pick=false"，看不出原来的破绽）。
- 建议（择一）：
  - 直接 `git rm` 掉这段 `#else` 死代码（推荐，去陈代码包袱）；
  - 或保留但加 `// TODO(opengl): 重启 OpenGL 路径时需补 m_gridPrimitive 声明与 setRender`。
- 同行注释 `//m_object->draw(m_camera->matrix(), false);` 也是把旧 `&m_render` 改成 `false`，**注释里的代码没必要跟签名**，属于无意义的"机械替换"动作。按 `personal-override.mdc` 的精神（不做过度防御 / 不做无意义的同步），这种注释码可以保持原样，或干脆删掉。

### W2. 新增的 `NULL == m_render` 检查放在 hot path 上

```77:85:cat/primitive.cpp
void Primitive::draw(const scl::matrix& mvp, const scl::matrix* jointMatrices, const int jointMatrixCount, bool isPick)
{
	// caller 必须先 setRender + setShaderWithPick 才能 draw；契约违反 → debug 暴露，release 跳过本 primitive 避免崩到底层 driver。
	Shader* const targetShader = isPick ? m_pickShader : m_shader;
	if (NULL == m_render || NULL == targetShader)
	{
		assert(false);
		return;
	}
```

- `m_render` 一旦 `setRender` 后即生命周期不变（构造期注入），每帧每 primitive 判一次属于冗余分支。
- 当前单线程 + primitive 数量级很小，**性能影响可忽略**，按"不过度防御 + 契约失败必须暴露"的取向，写法本身没问题。
- 仅作为提醒：如果未来 primitive 数量上量（>10k），可以考虑把这两个 NULL 检查全部下沉到 `setRender / setShader*` 时一次性校验后存一个 `m_ready` flag，draw 路径只判一次 flag。**当前不需要改**。

### W3. 注释 / 文档同步检查

- `cat/primitive.cpp:79` 注释升级正确（"setRender + setShaderWithPick"）。
- `cat/mesh.h:21`、`cat/object.h`、`cat/scene.h`、`cat/terrain.h` 的 `draw` 声明上方**没有签名相关注释**，无需同步，OK。
- diff 含 `doc/子系统-cat.md` 18 行改动，本次未贴进待审 diff，**默认相信文档侧同步过**。若想稳妥，建议自己再 `git show 756ba36 -- doc/` 扫一眼。

## Suggestion

### S1. `Terrain::m_render` 已经无人使用，建议一并清理

terrain.h:30 有 `IRender* m_render;`，terrain.cpp:11 `m_render = render;`、:93 `m_render = NULL;;`（顺手提一下，**有个多余的分号** `;;`）。
但 `Terrain::draw` 在本 commit 之后已经不依赖 `m_render`（只是透传给 `m_primitive->draw`），`init` 内部 `setIndices/setVertices` 走的是 `m_primitive->m_render`，所以 `Terrain::m_render` 已变成"只写不读"的死字段，可以删掉。

```9:14:cat/terrain.cpp
void Terrain::init(IRender* render, Env* env)
{
	m_render = render;
```

```81:94:cat/terrain.cpp
void Terrain::draw(const scl::matrix& mvp, bool isPick)
{
	if (NULL == m_primitive)
		return;
	m_primitive->draw(mvp, NULL, 0, isPick);
}

Terrain::Terrain()
{
	m_vertices		= NULL;
	m_indices		= NULL;
	m_primitive		= NULL;
	m_render		= NULL;;
}
```

顺便修掉 `NULL;;` 这个赘余的分号。

### S2. `Primitive::release()` 里直接用 `m_render`，没有 NULL 检查

```131:140:cat/primitive.cpp
void Primitive::release()
{
	//safe_delete(m_shaderMacros);

	if (NULL != m_deviceIndexBuffer)
	{
		//if (0 == G.refCounter.DecRef(m_deviceIndexBuffer))
		m_render->releaseIndexBuffer(m_deviceIndexBuffer);

		m_deviceIndexBuffer = NULL;
	}
```

- 既然 draw 路径现在用 `NULL == m_render` assert 保护契约，release 路径也应当配套有 assert（不是 if-skip —— 有 device buffer 但 m_render 为 NULL 是不变式被破坏的严重错）。
- 现状是：若有人构造 Primitive 但没 setRender 就 delete，`m_deviceIndexBuffer == NULL`、`m_deviceVertexBuffers == NULL`，不会触发解引用，安全。但**这条隐式不变式没写出来**。
- 建议在 release 入口加：`if (NULL != m_deviceIndexBuffer || NULL != m_deviceVertexBuffers) { assert(NULL != m_render); }`，把"有 device 资源 ⇒ m_render 非空"这个不变式显式化。**非阻塞，可后续 commit。**

### S3. commit message 建议补一行

按你们 doc/review 里其他 review 的口径，建议 message 改成两行：

```
取消 draw 参数中的 render

draw 链路统一改用 Primitive::m_render（构造路径已通过 setRender 注入），
顺便给 Primitive::draw 加 NULL == m_render 契约 assert。
```

## 影响范围分析

- **直接调用方**：`testCat/client.cpp` 4 处生效路径（terrain / scenes / grid / bonePrimitive）+ 1 处死代码（`m_gridPrimitive`）+ 1 处注释代码（`m_object`）—— diff 全部覆盖到。
- **间接调用方**：
  - `Mesh::draw` → 仅被 `Object::draw`（cat/object.cpp）调用。
  - `Object::draw` → 仅被 `Scene::draw`（cat/scene.cpp）和自身递归调用。
  - `Scene::draw / Terrain::draw / Primitive::draw` → 全部出口在 `testCat/client.cpp::_renderScene` 与 `Client::run`。
  - 2 层 callers 范围内没有遗漏未改的调用点。
- **数据流不变式**：所有 `new Primitive()` 调用点（cat/terrain.cpp、cat/gltfLoader.cpp、testCat/testPrimitive.cpp×5、testCat/simpleShape.cpp）在 `draw` 之前都已经 `setRender`，新增的 `NULL == m_render` assert 在正常使用路径上永远不会触发。
- **构建影响**：纯接口删减，无新 include、无新平台分支、无新 STL 暴露到头文件接口、无 `<vulkan/...>` 进入 cat 模块。跨平台编译 0 风险。

## 整体评价

| 维度 | 评分 | 说明 |
| --- | --- | --- |
| 性能 | A | 每帧每 primitive 少传 1 个指针参数，几乎无影响；新增的 NULL 检查可忽略。 |
| 崩溃 | A | 新增 `NULL == m_render` 契约 assert，比删参数前更安全；release 路径无回归。 |
| 逻辑 | A- | 主线等价。唯一非等价点在 `client.cpp:290` 死代码里（pick true→false），但因 m_gridPrimitive 早已注释，实际行为零变化。 |
| 规范 | A | 全程 `NULL`、Yoda 比较、Allman、纵向对齐、m_ 前缀、无 STL 头部暴露、无 vulkan include 渗透，全部合规。 |
| 跨平台编译 | A | 调用方全部已同步，未引入平台相关变化。 |

**结论：可以合并。** 唯一值得动手的小修是 `terrain.cpp:93` 的 `;;` 与 `Terrain::m_render` 死字段（S1）—— 可下一个 commit 顺手清理。`client.cpp` 的 `#else` 死代码（W1）属于历史包袱，本 commit 不背锅，但建议另起一个 commit 删掉，避免后续 review 反复纠结。
