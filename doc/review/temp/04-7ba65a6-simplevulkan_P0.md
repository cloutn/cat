# 代码审查 — 7ba65a6 simplevulkan P0 崩溃修复

审查范围：`catvulkan/cat/simplevulkan.cpp`
- `_createTextureImageFromFile` fopen NULL 守卫
- `_getLayoutBindsFromShader` push constant 块的循环误写
- `_createShaderFromFile` fopen NULL 守卫
- `svkAcquireNextImage` MAX_RETRY=5 + abort

引擎/层约定：`simplevulkan` 是 catvulkan 内部 C 风格薄封装层，`printf` / `abort` 允许使用；
`scl/log.h` 在本文件中**未** include（`vulkanRender.cpp` 才用 `log_xxx`），保持 C 风格无外部 cat 依赖是有意为之。
故下文不再对 printf 本身扣分。

---

## 总体结论

| 修复点 | 修法是否对 | 是否引入新问题 | 等级 |
| --- | --- | --- | --- |
| `_createShaderFromFile` fopen NULL 早退 | ✅ | 下游 pipeline 创建失败但不崩 | OK |
| `_getLayoutBindsFromShader` push constant 删 for 留 if | ✅ | 无 | OK |
| `svkAcquireNextImage` MAX_RETRY + abort | ⚠️ 部分对 | MAX_RETRY=5 偏小、abort 太早、未区分 fatal/recoverable | **P1** |
| `_createTextureImageFromFile` fopen NULL 早退 | ⚠️ 不完整 | **下游 `svkCreateTexture` 仍会拿 image=NULL 走 `vkCreateImageView`，假 fix** | **P0** |

---

## 五专家分头看

### 1. 崩溃专家

#### P0 — `_createTextureImageFromFile` 早退后 `svkCreateTexture` 仍会用 image=NULL

仅看 `_createTextureImageFromFile` 内部，早退合理，没有需要 free 的资源（fopen 失败 → 啥都还没分配，
`_createTextureImage`、`vkMapMemory`、`new buf` 都还没发生）。**fopen 之前没有任何资源分配，所以"return 之前是否漏关其他资源"答案是 否，单函数无泄漏。**

但跨函数看就崩：

```1550:1576:catvulkan/cat/simplevulkan.cpp
svkTexture svkCreateTexture(svkDevice& device, const char* const filename, VkCommandBuffer outCommandBuffer, svkLoadImageDataCallback loadDataCallback)
{
	const VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;

	svkTexture _svkTexture;
	memclr(_svkTexture);
	//VkResult err;

	_createTextureImageFromFile(device, filename, &_svkTexture, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, loadDataCallback);

	// Nothing in the pipeline needs to be complete to start, and don't allow fragment // shader to run until layout transition completes
	VkCommandBuffer commandBuffer = outCommandBuffer;
	if (NULL == outCommandBuffer)
	{
		...
	}

	_createSamplerAndImageView(device, _svkTexture, texFormat);

	return _svkTexture;
}
```

文件不存在时：
1. `_createTextureImageFromFile` 早退，`_svkTexture` 仍是 memclr 后的 0 状态（image = VK_NULL_HANDLE，width=0，height=0）。
2. `svkCreateTexture` **不检查返回**，继续录制 layout transition（`_setImageLayout` on VK_NULL_HANDLE，validation error），并调 `_createSamplerAndImageView`。
3. `_createSamplerAndImageView` 内部 `vkCreateImageView(image=VK_NULL_HANDLE)` ——
   - debug 层：VUID-VkImageViewCreateInfo-image-parameter validation error；
   - 紧接着 `_CreateColorImageView` 末尾 `assert(!err);` 在 debug build 触发；
   - release build：`view` 未初始化即返回，被写入 `_svkTexture.view`；后续 `svkDestroyTexture` → `vkDestroyImageView(garbage)` 崩。

**也就是说，原崩点（fclose(NULL)/stb_image NULL）确实修了，但崩点向后挪了 ~3 个调用，并且在 release 下变成"silent garbage handle"，比原崩更难定位。**

**建议（任选其一，按代价从小到大）**：

- 最小成本：在 `svkCreateTexture` 中早退守门：
  ```cpp
  _createTextureImageFromFile(...);
  if (VK_NULL_HANDLE == _svkTexture.image)
      return _svkTexture;	// 全 0 svkTexture，调用方根据 image==NULL 判失败
  ```
  并要求所有 `svkCreateTexture` 调用方按 `texture.image == NULL` 判错（grep 一下 `svkCreateTexture(` 用法）。

- 更彻底：把 `_createTextureImageFromFile` 改 `bool` 返回，`svkCreateTexture` 也改 `bool svkCreateTexture(..., svkTexture* out)`（既然异常禁用，返回值是约定的错误通道）。

---

#### P1 — `svkAcquireNextImage` MAX_RETRY=5 + abort()

针对用户三个具体提问：

**Q1：MAX_RETRY=5 是否太小？**
**偏小。** Windows 实战中以下场景容易连续 ≥3 次 OUT_OF_DATE / SUBOPTIMAL：

- 用户拖动窗口边缘高速 resize（每帧 surface 都变），acquire → callback recreateSwapchain → 下一帧 acquire 又拿到旧 size → 又 OUT_OF_DATE。
- 多显示器拖拽过程的 DPI 切换 + OS 通知重叠。
- 最小化↔恢复连击：`recreateSwapchain` 内部 `if (_minimized()) return;` 早退**不重建** swapchain，下一轮 acquire 拿到的还是旧 swapchain，又 OUT_OF_DATE。这条路径可以稳定撞 MAX_RETRY 然后 abort，**实际上把"用户最小化"误判成"驱动 fatal"。** 这是真 bug。
- `VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT`（全屏切换）。

**Q2：abort() 是否合适？**

不合适，原因有两个：

- 没区分错误码。`VK_ERROR_DEVICE_LOST` / `VK_ERROR_OUT_OF_DEVICE_MEMORY` / `VK_ERROR_INITIALIZATION_FAILED` 才是真不可恢复，应该尽早 fatal；`OUT_OF_DATE_KHR` / `SURFACE_LOST_KHR` / `SUBOPTIMAL_KHR` 是 spec 允许的"重建重试"路径，不该有上限。
- abort() 走 C runtime，跳过 cat 的析构/日志 flush。simplevulkan 层允许 abort，但落到这里 **abort 之前先 printf 是好的，但不够**：调用方（`VulkanRender::beginDraw`）拿不到任何 hint。建议至少 callback 一次 fatal 通知，让 `vulkanRender.cpp` 有机会 dump 一些上下文。

**Q3：是否应当传给 callback 让上层处理？**

更合适的模式（不必本次改，记 backlog）：
```cpp
// 区分错误码
switch (err)
{
case VK_ERROR_OUT_OF_DATE_KHR:
case VK_ERROR_SURFACE_LOST_KHR:
case VK_SUBOPTIMAL_KHR:
    // 可恢复，无上限重试，但每次给 callback 一次重建机会
    if (NULL != callback) callback(userData, err);
    break;
case VK_ERROR_DEVICE_LOST:
case VK_ERROR_OUT_OF_DEVICE_MEMORY:
case VK_ERROR_OUT_OF_HOST_MEMORY:
    // 立刻 fatal
    printf("svkAcquireNextImage fatal: err=%d\n", (int)err);
    abort();
default:
    // 可疑，给个上限
    if (++retry >= MAX_RETRY) abort();
    break;
}
```

或者保留当前粗粒度结构但 **MAX_RETRY 至少抬到 32~64**，+ 对 DEVICE_LOST 立即 abort。当前 5 太狙击用户操作了。

另：beginDraw 中 `svkAcquireNextImage` 的返回是直接写进 `m_frameIndex` 的，目前函数永不返回 -1（do-while 直到成功），这是 caller 的隐式契约。如果改成"放弃当帧返回 -1"，caller 的 `m_frameUniforms[m_frameIndex]` 必须同步加 `m_frameIndex >= 0` 守门。**改不改都行，但要么都不改、要么一起改，不能只改 simplevulkan。**

---

#### `_createShaderFromFile` 早退

```741:766:catvulkan/cat/simplevulkan.cpp
static VkShaderModule _createShaderFromFile(svkDevice& device, const char* const filename, ...)
{
#pragma warning(push)
#pragma warning(disable:4996)
	FILE* f = fopen(filename, "rb");
#pragma warning(pop)
	if (NULL == f)
	{
		printf("Error opening shader file: %s\n", filename);
		return NULL;
	}

	const int	BUF_SIZE	= 1024 * 1024;
	char*		buf			= new char[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);
	int len = fread(buf, 1, BUF_SIZE, f);

	VkShaderModule shaderModule = _createShaderFromCode(...);
	fclose(f);
	delete[] buf;

	assert(NULL != shaderModule);
	return shaderModule;
}
```

OK。fopen 之前 buf 还没 new，无泄漏。返回 NULL 与 `_createShaderFromCode` shaderc 失败的现有 NULL 路径一致；`_buildShaderStageCreateInfo` 已对各 stage NULL 做 `if (NULL != prog.vert)` 判断，不会崩。下游会落到 `vkCreateGraphicsPipelines` validation error，比 abort 友好。这条算修对。

---

### 2. 逻辑专家 — push constant 修法

**SPIRV-Cross 的 `SPVC_RESOURCE_TYPE_PUSH_CONSTANT` 每 stage 真的最多 1 个吗？**

是。SPIR-V 规范限定：**一个 entry point（≈ 一个 shader stage module）至多 1 个 PushConstant 存储类的接口变量**（OpEntryPoint 的 interface 集合中 PushConstant OpVariable 至多 1 个），SPIRV-Cross `get_resource_list_for_type(PUSH_CONSTANT)` 因此返回 0 或 1 项。所以新代码**逻辑上正确**：删 for、留 if、用 list[0] 是匹配 spec 的。

#### 旧代码到底什么表现（澄清注释）

```cpp
for (size_t i = 0; i < count; i++)
if (count > 0)
{
    ... list[0] ...
}
```

按 C++ 语法这等价于：
```cpp
for (size_t i = 0; i < count; i++) {
    if (count > 0) {
        ... list[0] ...
    }
}
```

也就是 if 块就是 for 的 body。当 count == 1 时只跑 1 次（正确），当 count == 0 时不跑（正确）。
**只有当 count >= 2 时才会出现"用 list[0] 重复填 count 次直至 capacity 溢出 + assert + 提前 return"。**

但前面证明了 count ≤ 1，所以**旧代码在合规 SPIR-V 输入下其实从不踩坑**。
该修复属于"逻辑写错但没触发的潜在炸弹拆除"，不是"已经在崩的真 P0"。
**修对了，但 commit 把它列为 P0 偏夸张**，且 commit message 里"count>1 时溢出"的描述需要前置条件"非合规 SPIR-V"。建议提交说明里改写为"防御性纠正：循环写法等价于多次填入相同 list[0]，当未来 SPIRV-Cross 行为变更或非合规输入时会溢出"。

#### 同函数另一遗留问题（preexisting，不算本 commit 退化但顺手提）

`_getLayoutBindsFromShader` 三处 `assert(false); return;`（uniform_buffer 行 595、sampled_image 行 617、push_constant 行 645）都**跳过函数末尾**：

```666:667:catvulkan/cat/simplevulkan.cpp
	spvc_context_release_allocations(context);
	spvc_context_destroy(context);
```

这意味着 capacity 配置错误时 spvc context 泄漏。Commit message 自夸"原代码漏掉 spvc 清理"，但**新代码 645 行的 return 同样漏 spvc 清理**，这条没改。建议下一次顺手把三个 early return 包到 `goto cleanup` 或者用 RAII。**不是本 commit 引入，所以不纳入 P0 评级，但 commit message 描述与现实不符。**

---

### 3. 性能专家

无热路径影响：

- 三处守卫都是 cold path（启动期一次性资源加载 / 极少触发的 surface 错误）。
- printf 在错误路径才走，无 spam 风险。
- `MAX_RETRY=5` 改大到 32/64 也无性能成本（只在错误路径循环）。

---

### 4. 规范专家

| 项 | 状态 |
| --- | --- |
| NULL（不用 nullptr） | ✅ 全用 NULL |
| Yoda 比较 `NULL == f` | ✅ |
| Allman | ✅ |
| Tab + 纵向对齐 | ✅ |
| `MAX_RETRY` 局部常量大写 | ✅ |
| `printf` / `abort` 在 simplevulkan | ✅ 允许 |
| `#pragma warning push/pop` 包 fopen | ✅ |
| 注释中文，解释 why | ✅ |
| 异常禁用 → 返回值 + assert | ✅ 早退用返回；abort 是 C 风格 fatal |

唯一吐槽（**风格小事，不强求**）：`MAX_RETRY` / `retry` / `err` / `nextFrame` 用了纵向对齐和 `(uint32_t)-1`，OK；
但 `nextFrame = (uint32_t)-1` 不是错误哨兵（函数最终保证返回的是成功值），看起来是防御性初始化。如果就只是为了不被未初始化警告，可以加注释 `// 仅为静态分析消警`。

---

### 5. 跨平台编译专家

- `fopen` + `#pragma warning(disable:4996)` 是 MSVC-only；Linux/Mac 编译 `simplevulkan` 时 #pragma warning 被 GCC/Clang 忽略（带 `-Wpragmas` 也只是 warning），不影响编译。本 commit 没新增平台相关 API，OK。
- `printf` 在 `<stdio.h>`，全平台。`abort()` 在 `<stdlib.h>`，全平台。本文件顶部应已 include；现有代码 257 行附近就在用 fopen，说明 stdio.h 已 include。
- `(uint32_t)-1` 在所有平台行为一致（implementation-defined 但 unsigned 转换有规范定义结果 0xFFFFFFFF），OK。
- 无跨平台编译问题。

---

## 必修动作（按优先级）

### P0
1. **`svkCreateTexture` 必须检查 `_createTextureImageFromFile` 后的 `_svkTexture.image`**，否则当前 commit 的 fopen 守卫只是把崩点从 `fclose(NULL)` 推到 `vkCreateImageView(NULL_HANDLE)` + `_CreateColorImageView` 末尾的 `assert(!err)`，release build 还会带回一个含未初始化 view 的 svkTexture 出去，下次 destroy 时崩。

   建议改法（最小侵入）：
   ```cpp
   _createTextureImageFromFile(device, filename, &_svkTexture, ...);
   if (VK_NULL_HANDLE == _svkTexture.image)
       return _svkTexture;	// 调用方按 image==NULL 判失败
   ```
   同时需要 grep 所有 `svkCreateTexture(` 调用处，确认其能容忍 image == NULL（很可能需要补判）。

### P1
2. `svkAcquireNextImage`：
   - 把 MAX_RETRY 从 5 抬到 ≥ 32，避免 resize / minimize 风暴误杀；
   - 或更好：按 err code 分流——`VK_ERROR_DEVICE_LOST` / `OUT_OF_DEVICE_MEMORY` / `OUT_OF_HOST_MEMORY` 立即 abort（这才是真 fatal），`OUT_OF_DATE_KHR` / `SURFACE_LOST_KHR` / `SUBOPTIMAL_KHR` 走无上限 callback 重试。

### P2（不阻塞本 commit 合入）
3. `_getLayoutBindsFromShader` 中三处 `assert(false); return;` 漏 `spvc_context_destroy`，建议下一次清理（preexisting，commit 7ba65a6 没引入也没修复）。
4. commit message 里"push constant count>1 时溢出"的描述请补"在非合规 SPIR-V 输入下"前提，避免读者误以为合规输入也会触发。

---

## 一句话给提交者

push constant 和 shader fopen 两处修对了；texture fopen 守卫**只修了一半**——下游 `svkCreateTexture` 必须同步判 `image == NULL`，否则崩点只是从 stb_image/fclose 挪到 vkCreateImageView，release 还会埋下"含未初始化 view handle 的脏 svkTexture"这种更难查的雷。`MAX_RETRY=5` 太狙最小化/resize 风暴，建议 ≥32 或按 err code 分流，`abort()` 留给真 fatal（DEVICE_LOST / OOM）。
