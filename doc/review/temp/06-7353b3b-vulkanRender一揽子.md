# vulkanRender 一揽子修复审查（commit 7353b3b）

范围：`catvulkan/cat/vulkanRender.{cpp,h}` + `catbase/cat/IRender.h`（删除 `createShader(int)`）
角色：性能 / 崩溃 / 逻辑 / 规范 / 跨平台编译 5 视角合审
工具调用：4（cpp 全读 + h 全读 + 2 条 grep）

---

## 结论一句话

主线方向正确（spec 顺序销毁 / 预算守门 / 死代码与死接口清理）。**但 `recreateSurface` 的 `_minimized()` 早退路径会留下悬垂 swapchain，下一次 `swap` 必崩**；`_fillDynamicOffsets` 失败路径返回 0 后 `draw2` 不做兜底，release 下会发出层数不匹配的 `vkCmdBindDescriptorSets`，最终 device lost。其余修复均可合入。

---

## 🔴 Critical（必须修）

### C1. `recreateSurface` 早退后留下半销毁状态 → 下一次 `swap` 必崩

位置：`catvulkan/cat/vulkanRender.cpp:1308-1328`

```1308:1328:catvulkan/cat/vulkanRender.cpp
void VulkanRender::recreateSurface()
{
	// Vulkan spec: 依附于 VkSurface 的 VkSwapchain / VkFramebuffer / VkImageView 必须先于 surface 销毁。
	// 此函数把"销毁旧资源 → 重建 surface → 重建主渲染目标"做成原子序列，调用方不再单独 recreateSwapchain
	svkDeviceWaitIdle(m_device);

	_destroyMainRenderTarget();
	_destroyRenderTarget(m_device, m_pickRenderTarget);

	svkDestroySurface(m_inst, m_device, m_surface);
	m_surface = svkCreateSurface(m_inst, m_device, m_windowInstance, m_windowHandle);
	svkRefreshSurfaceSize(m_device, m_surface);

	if (_minimized())
		return;
	// ...
}
```

走位：`presentCallback(SURFACE_LOST)` → `recreateSurface` → 新建 surface 后窗口正好处于 minimized（SURFACE_LOST 常和 win-min/destroy 共同出现）→ 早退。此时：

- `m_swapchain / m_frames / m_mainRenderPass / m_mainDepthImage / m_pickRenderTarget` 已销毁
- `_destroyMainRenderTarget` 末尾**未 memclr**，handle 仍是旧值（悬垂）

后续：

1. 下一帧 `swap()` → `_minimized()` 为真 → `recreateSwapchain()` → 内部 `_destroyMainRenderTarget()` **第二次 destroy** 已被销毁的 handle → 大多数 driver UB / validation error / 崩。
2. 即使靠 driver 容错混过去：当窗口恢复，`swap()` 走 `svkPresent(... m_swapchain ...)` 时 `m_swapchain` 仍是悬垂（`recreateSwapchain` 在 minimized 时也会早退） → 崩。

注：commit message "recreateSurface 内部已含 swapchain 重建" 在 minimized 早退路径不成立，`presentCallback` 现在依赖了不存在的保证。

**修法（任选一种）**：

- **A. 推荐**：先创建新 surface + refreshSize，**若 minimized 就保留旧资源直接 return**（旧 swapchain 失效但不去 destroy，下次 unminimize 后正常走 recreateSwapchain，由它统一销毁+重建）。
- **B. 早退路径把所有 main/pick handle 显式 `memclr` 清零**，并修 `_destroyMainRenderTarget` 让所有 destroy 后写 NULL；同时让 `svkPresent / recreateSwapchain` 全链路对 NULL handle 健壮。

### C2. `_fillDynamicOffsets` 预算耗尽返回 0，但 `draw2` 不检查 → release 下静默 device lost

位置：`catvulkan/cat/vulkanRender.cpp:918-966`、`1230-1245`

预算守门触发时：

```925:947:catvulkan/cat/vulkanRender.cpp
	if (dynamicOffsetCapacity < 2)
	{
		assert(false);
		return 0;
	}

	if (m_frameDrawCount >= MAX_DRAW_PER_FRAME)
	{
		// 单帧 draw 数超过 MAX_DRAW_PER_FRAME，需调大上限或减少 draw call
		assert(false);
		return 0;
	}
```

`draw2` 拿到 `dynamicOffsetCount = 0` 后**直接继续**：

```1230:1245:catvulkan/cat/vulkanRender.cpp
	_prepareDescriptorSetAndFillData(shader, mvp, texture, jointMatrices, jointMatrixCount, descriptorSet, descriptorSetLayout, dynamicOffsets, countof(dynamicOffsets), dynamicOffsetCount);

	svkPipeline* pipeline = NULL;
	_preparePipeline(primitiveType, attrs, attrCount, vertexBuffers, descriptorSetLayout, shader, m_drawContext.renderPass, pipeline);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);	
	
	vkCmdBindDescriptorSets(
		cmd_buf, 
		VK_PIPELINE_BIND_POINT_GRAPHICS, 
		pipeline->layout, 
		0, 
		1,
		&descriptorSet.set, 
		dynamicOffsetCount,    // <-- 0
		dynamicOffsets);
```

Release 下 assert 是空操作 →

1. `dynamicOffsetCount = 0` 但 descriptor set layout 里至少有 1 个 dynamic UBO（mvp）。`vkCmdBindDescriptorSets` 规范要求 `dynamicOffsetCount == 该 set 内 dynamic descriptor 数量`，否则 `VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359` → validation error；
2. 即使 validation 关掉，dynamic offset 取默认 0，多 draw 共用 offset 0 → 渲染错乱；如果 cache 命中的 descriptorSet 是别的 layout（hash key 包含 dynamicOffsetCount，未必命中），可能拿到任意数据 → device lost。
3. `descriptorSet.set` 在 `_prepareDescriptorSet` 早退分支可能是 `{0}` 未初始化 → `vkCmdBindDescriptorSets` 喂 NULL handle → 直接崩。

**修法**：`_fillDynamicOffsets` 返回 0 时 `draw2` 必须早退；推荐用 sentinel：

```cpp
if (0 == dynamicOffsetCount)
{
    // 失败已在 _fillDynamicOffsets assert，release 下直接跳过本 draw
    return;
}
```

同时 `_prepareDescriptorSetAndFillData` 也应在 `outputDynamicOffsetCount == 0` 时不调 `_prepareDescriptorSet`，避免 cache 污染（用 dynamicOffsetCount=0 当 key 会塞进 cache）。

---

## 🟠 High（强烈建议）

### H1. `_prepareDescriptorSet` 守门 assert 用 `< MAX×MAX_CONFLICT×2` 太松

位置：`vulkanRender.cpp:873-874`

```873:874:catvulkan/cat/vulkanRender.cpp
		// 守门：cache 只增不减；一旦触发 hash_table::_grow（capacity ×16），说明 cache 已无界增长，需要按场景 boundary 做 evict
		assert(m_descriptorSetCache.capacity() < MAX_DESCRIPTOR_SET_CACHE_SIZE * MAX_CONFLICT * 2);
```

`scl::hash_table::_grow` 通常 ×2（请实际确认 scl 容器倍率，注释说 ×16），守门以 ×2 作为门槛意思是"已经 grow 过一次就 fire"。OK，但：

- 该 assert 是**事后**：grow 已发生，descriptor set 数量已经爆，泄漏发生在前。建议在 `find_index` 返回 -1 即将 add 之前，先比较 `size() < MAX_DESCRIPTOR_SET_CACHE_SIZE` 给出**事前**警告。
- release 下 assert 失效，cache 仍会一直 grow，DescriptorPool 也会被 alloc 到耗尽（VkDescriptorPool 容量取决于 `DescriptorAllocator` 内部上限）。建议在 `assert(false)` 旁加一个 throttled `scl::log_error_unsafe("descriptor set cache overflow")`，方便线上排查。

### H2. `_minimized()` 在 `recreateSwapchain` 与 `recreateSurface` 行为差异 + `m_onSurfaceResize` 未通知

`recreateSwapchain`（vulkanRender.cpp:1289-1306）：minimized 时**不通知** `m_onSurfaceResize`、不销毁、不重建。OK。
`recreateSurface`：minimized 时**销毁**但不通知、不重建。如 C1 所述。

`m_onSurfaceResize` 的语义本应在尺寸真正变化时通知 client（client 可能用它重算相机/UI 布局）。minimized → width=0 时 client 是否要收到这次通知？现在两者都不通知，client 维持上次的非零尺寸 → 可能用一个旧的 viewport。**建议**：min/unmin 边界都通知 client，让 client 自己决定是否要响应 0 尺寸。

### H3. `recreateSwapchain` 中 `svkDeviceWaitIdle` 与 `recreateSurface` 中 `svkDeviceWaitIdle` 重复执行不会冲突，但 `waitIdle()` 已在 `presentCallback` 调过

位置：`vulkanRender.cpp:497-503` + `1296` + `1312`

```497:503:catvulkan/cat/vulkanRender.cpp
	if (err == VK_ERROR_OUT_OF_DATE_KHR) 
	{
		render->waitIdle();
		render->recreateSwapchain();
	}
	else if (err == VK_ERROR_SURFACE_LOST_KHR) 
	{
		render->waitIdle();
		render->recreateSurface();	// recreateSurface 内部已含 swapchain 重建，不再单独调 recreateSwapchain
	}
```

`waitIdle()` 已经 `svkDeviceWaitIdle`，`recreateSwapchain/recreateSurface` 内部又 wait 一次。功能正确（idempotent），但语义重复。建议二选一：要么去掉 `presentCallback` 里的 `waitIdle`，要么去掉函数内的；推荐**留在函数内**，因为 `recreateSurface/recreateSwapchain` 也会被外部直接调（如 `swap` 处理 minimized），不能假设调用方已 wait。

---

## 🟡 Medium（合入前最好处理）

### M1. `beginPickPass` 与 `beginScenePass` 不复位 `m_frameDrawCount`，pick 与 scene 共享预算

`beginDraw` 里复位 `m_frameDrawCount = 0`（vulkanRender.cpp:578），pick / scene 共用同一个 `m_frameUniforms[m_frameIndex]` 和 `m_frameUniformBufferOffset`，所以**预算共享是正确的**——这点没问题。

但要在 header 注释里写明"pick + scene 合计 ≤ MAX_DRAW_PER_FRAME"，否则下个维护者看到 `MAX_DRAW_PER_FRAME=1024` 会以为是"scene draw"独占的额度。建议把注释从

```225:225:catvulkan/cat/vulkanRender.h
	static const int	MAX_DRAW_PER_FRAME				= 1024;	// 每帧最大 draw 调用数（_fillDynamicOffsets 入口 assert）
```

改为：

```cpp
	static const int	MAX_DRAW_PER_FRAME				= 1024;	// 每帧 pick + scene draw 合计上限；超出会在 _fillDynamicOffsets assert 并跳过 draw
```

### M2. `_fillDynamicOffsets` 预算公式与 init 里的计算应抽出常量

init 里算 `perDrawBytesMax`、`_fillDynamicOffsets` 又算 `mvpStride/jointStride`，公式漂移风险高（之前的 16MB→4.25MB 就是漂移的典型）。

每次 draw 都重算 `_alignUniformBufferOffset(sizeof(mvp))`：`sizeof(mvp)` 编译期常量，`minUniformBufferOffsetAlignment` 设备常量。建议在 init 时算一次 `m_mvpStride`（成员）并复用，与 `m_frameUniformBudget` 一起持有。当前每帧 1024 draw × 几条整除指令是小钱，但代码可读性 / 防漂移更值。

### M3. `_fillDynamicOffsets` 失败路径在 release 没有任何日志

```cpp
if (m_frameUniformBufferOffset + mvpStride + jointStride > m_frameUniformBudget)
{
    assert(false);
    return 0;
}
```

assert 是 debug-only。release 下静默吃掉 draw 是逻辑黑洞，难定位。建议加一次性 throttled log：

```cpp
{
    static bool s_logged = false;
    if (!s_logged)
    {
        scl::log_error_unsafe("uniform budget exhausted: drawCount=%d offset=%u budget=%u",
            m_frameDrawCount, m_frameUniformBufferOffset, m_frameUniformBudget);
        s_logged = true;
    }
    assert(false);
    return 0;
}
```

### M4. `savePickPass` 注释关于 fence 不 reset 的依赖

```669:671:catvulkan/cat/vulkanRender.cpp
	// svkWaitFence 只 wait 不 reset (simplevulkan.cpp:1041)，依赖下一次 svkQueueSubmit (simplevulkan.cpp:2214) 在 submit 前 reset。
	// 即使 endPickPass 中途 early-out 不 submit，fence 保持 signaled 也只会让下一次 beginPickPass 进入，逻辑安全
	svkWaitFence(m_device, &m_pickFence, 1);
```

注释正确，但忽略一个情形：`recreateSwapchain / recreateSurface` 时 `svkDeviceWaitIdle` 把 GPU 跑完，pick fence 进入 signaled 状态。之后旧 fence 没 reset、新 swapchain 起来 → 下一次 `beginPickPass` 立刻通过 `svkIsFenceSignaled` 检查、覆盖 `m_drawContext`、调 `endPickPass` → `svkWaitFence` 立即返回 → `savePickPass` 读到的 `m_pickPassImageCPUBuffer` 是**旧帧内容**（recreateSwapchain 期间没 copy 过新数据）。

实际影响：picking 返回上一帧颜色一次，不致命，但 UI/交互层可能误判。**修法**：`recreateSwapchain / recreateSurface` 结尾 reset 一下 pick state（或在 `savePickPass` 加 first-frame guard）。

### M5. `drawIMGUI` 补 `renderPass` 早退是兜底，但根因没修

```1337:1344:catvulkan/cat/vulkanRender.cpp
void VulkanRender::drawIMGUI(ImDrawData* draw_data)
{
	if (_minimized())
		return;

	if (NULL == m_drawContext.renderPass)
		return;
```

commit 注释说"drawIMGUI 在 endScenePass 之后调用就崩"。早退避免崩没问题，但**掩盖了 client 调用顺序错的 bug**。建议加 debug-only 提示：

```cpp
if (NULL == m_drawContext.renderPass)
{
    assert(false && "drawIMGUI must be called between beginScenePass and endScenePass");
    return;
}
```

让开发期 client 立刻知道顺序错；release 兜底不崩。这条同样适用于 `draw2:1215` 的相同早退。

### M6. `endPickPass` 在 `m_drawContext.renderPass == NULL` 早退后**不 reset pickFence/不 submit copy CB**

```604:608:catvulkan/cat/vulkanRender.cpp
scl::vector4 VulkanRender::endPickPass(int x, int y)
{
	if (NULL == m_drawContext.renderPass)
		return scl::vector4{0};
```

如果 `beginPickPass` 因 `!svkIsFenceSignaled` 早退（vulkanRender.cpp:589-590），`m_drawContext.renderPass` 不会被赋值（保持上次 memclr 后的 NULL）→ `endPickPass` 早退返回 zero `vector4`。**客户端拿到 (0,0,0,0) 当 pick id** —— 如果你的场景中 0 是合法 id（很常见），就会误选。

建议：要么 `beginPickPass` 早退时**显式置 sentinel renderPass** 标记"this frame skipped"，`endPickPass` 返回明确 "no result"；要么在 client 层用 `bool tryPick(...)` 接口分离"有结果 / 无结果"。当前接口语义不安全。

---

## 🟢 Low / Nit

### L1. `~VulkanRender` `if (!m_isInit) return` 短路 OK，但 `m_isInit` 默认值是 false，构造抛异常时也安全。✓

注：cat 引擎不用异常，所以这条主要是防御 ctor 列表里某个 svk 调用走 NULL 路径的情况。✓

### L2. `memclr(m_device);` 重复删除 → 对，原代码三处 `m_device` 被 memclr 两次是 copy-paste 残留。`m_device.gpuProperties` 是 `VkPhysicalDeviceProperties`，全部 POD（含定长数组），`memclr` 对深嵌也清干净。✓

### L3. `safe_delete(pipelines[i])` 替换 `delete pipelines[i]`

`pipelines` 是 `scl::varray<svkPipeline*>`，`pipelines[i]` 返回 `svkPipeline*&`（lvalue），符合 `safe_delete` 宏对 lvalue 的要求。✓ 注意：循环结束后 `pipelines` 本身析构会释放数组（每个元素是裸指针副本，已经 safe_delete 置 NULL，重复释放无害）。✓

### L4. `MAX_OBJECT_PER_FRAME` → `MAX_DRAW_PER_FRAME` / `MAX_MATRIX_PER_FRAME` → `MAX_JOINT_PER_OBJECT` 改名很好

旧名误导（暗示是 object 数 × matrix 数），新名直接描述用途。✓

### L5. `_alignUniformBufferOffset` 仍是成员方法 + 用 `m_device.gpuProperties.limits.minUniformBufferOffsetAlignment`

热路径每 draw 调 1~2 次，乘除运算很轻量。如做 H2 提到的"init 时缓存 stride"，可彻底干掉。

### L6. `createShader(int)` 删除

`grep createShader\s*\(\s*int` 仅 `vulkanRender.cpp` 有同名实现已删，`IRender.h` 的 pure virtual 也同步删了。`catglesx / testCat` 等模块 grep 无其他实现。**删除安全，无 ABI 缺口**。✓

### L7. `presentCallback` 注释 "recreateSurface 内部已含 swapchain 重建" —— 见 C1，需要在修了 C1 之后才完全成立。

### L8. `_createMainRenderTarget` 把 `m_prevFrameIndex = 0`（而不是 -1）的注释 ✓

注释解释清晰，避免后人误以为应该和初始值一致。

### L9. DrawContext POD 注释 ✓

`endScenePass / endPickPass` 末尾 `memclr(m_drawContext)` 的前提是 POD，加注释防御未来误加 `std::function`。

### L10. 单线程约定注释 ✓

VulkanRender 类注释明示单线程；`presentCallback` 在调用线程同步触发，hash_table 不加锁的依据落地。

### L11. `recreateSwapchain` 加 `svkDeviceWaitIdle` 在 `_minimized()` 之后 ✓

正确顺序：先检查 minimized 早退（避免做无意义工作），不 minimized 才 wait。

### L12. `pickPassImageCPUBuffer` 4 字节 ✓

注释也补齐了"和 surface 尺寸无关、recreateSwapchain 不重建"。逻辑一致。

### L13. `draw2` `indexOffset >= 0` assert + 显式 `static_cast<VkDeviceSize>` ✓

注释解释清晰（"隐式转 VkDeviceSize 会变成 0xFFFFFFFFFFFFFFFF"），防御负值进 GPU。注：底层 `vkCmdBindIndexBuffer` 实际签名是 `VkDeviceSize offset`，所以是接口层防御。

### L14. `IRender.h` 同步删 `createShader(int)` 后**所有派生实现都已清理**。`catglesx` 等模块没出现该签名实现。✓

---

## 跨平台编译

- 头文件改动均不引入新依赖。`catbase/cat/IRender.h` 删 pure virtual 是源码兼容（任何旧的派生实现编译期会因"无 base 同名 virtual"导致 override 警告失败 → 但**所有派生类都是本仓库内的**，已同步删除）。
- 新增成员 `uint32_t m_frameUniformBudget; uint32_t m_frameDrawCount;` — `<cstdint>` 通过 `simplevulkan.h` 链路已包含。✓
- `static_cast<VkDeviceSize>(int)` 在 32 位平台 `VkDeviceSize` 仍是 `uint64_t`，提升不丢精度。✓

## 规范合规

| 项 | 符合 |
|---|---|
| `NULL` / Yoda / Allman / Tab | ✓ |
| `m_/s_` 前缀 | ✓ |
| `#pragma once` | ✓ |
| `cat` 模块禁 include `<vulkan/...>` | ✓（本 commit 改的全是 catvulkan） |
| `safe_delete` 替换 `delete` | ✓（pipelines 路径已替换，但 ~VulkanRender 中 `delete allocator;`、`delete m_commandAllocator[i];`、`delete m_pickCommandAllocator;` 仍是裸 `delete`——既有，不属本次新增，可后续清扫） |
| 末尾空行未改 | ✓（cpp 1622 行收尾、h 287 行收尾未动） |
| UTF-8 无 BOM | ✓ |

---

## 修复优先级清单

| Pri | 项 | 必修说明 |
|---|---|---|
| P0 | C1 recreateSurface minimized 早退悬垂 | swap → crash，必修 |
| P0 | C2 _fillDynamicOffsets 失败 draw2 不早退 | release 静默 device lost，必修 |
| P1 | H1 descriptor cache 守门时机 | 加事前 check 与日志 |
| P1 | H2 m_onSurfaceResize 通知策略 | client 行为依赖 |
| P1 | H3 waitIdle 双重调用 | 语义重复，去重 |
| P2 | M1 MAX_DRAW_PER_FRAME 注释 | pick + scene 合用预算 |
| P2 | M2 mvpStride 缓存到 init | 防止公式漂移 |
| P2 | M3 release 兜底日志 | 排障必要 |
| P2 | M4 recreateSwap 时 pick fence 状态 | 边界 1 帧脏数据 |
| P2 | M5 drawIMGUI 早退加 debug assert | 暴露 client 顺序错 |
| P2 | M6 endPickPass 返回 (0,0,0,0) 歧义 | API 语义不安全 |

C1 / C2 修了之后，本批可合入。

