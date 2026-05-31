# 阶段05-子审查-Vulkan-SimpleVulkan

子审查 agent：Godel `019e7e93-8892-7061-99e1-2da7e57070c9`

## Critical

### C1 acquire 失败后 recreate，重试仍使用旧 frame semaphore

- commit：`7353b3b` / `7ba65a6`
- 文件：`catvulkan/cat/vulkanRender.cpp:568` `beginDraw/endScenePass`，`catvulkan/cat/simplevulkan.cpp:2203` `svkAcquireNextImage`
- 风险：`vkAcquireNextImageKHR` 失败后 callback 会 `recreateSwapchain()` 并把 `m_prevFrameIndex` 重置为 0，但 `svkAcquireNextImage` 的局部 `frame` 仍是旧帧索引，重试时 signal 的 semaphore 可能是旧 `frames[frame].imageAcquireSemaphore`；随后 `endScenePass` 等的是 `m_frames[m_prevFrameIndex].imageAcquireSemaphore`。旧帧索引非 0 时会等待未 signal semaphore，GPU submit 可死等。
- 建议：不要在 `svkAcquireNextImage` 内部 callback 后隐式继续；让 acquire 返回 `VkResult + imageIndex + acquireSemaphoreIndex`，由 `beginDraw` 统一重启帧。至少 callback recreate 后同步更新用于 acquire / submit 的 semaphore index。

### C2 recreateSurface 最小化返回后保留已销毁句柄

- commit：`7353b3b`
- 文件：`catvulkan/cat/vulkanRender.cpp:1308` `VulkanRender::recreateSurface`，`catvulkan/cat/vulkanRender.cpp:1055` `_destroyMainRenderTarget`
- 风险：`recreateSurface()` 先销毁 main / pick render target，再创建新 surface；如果新 surface 尺寸为 0，`_minimized()` 直接 return。此时 `m_mainRenderPass`、`m_mainDepthImage`、`m_swapchain.swapchain` 等成员仍保留已销毁句柄，后续析构或再次 recreate 会 double destroy / 使用 stale handle。
- 建议：销毁 helper 必须清零 owner 成员并重置 `m_frameCount/m_frameIndex/m_prevFrameIndex`；`svkDestroySwapchain/svkDestroyImage/svkDestroyRenderPass` 成功后也应置空句柄。minimized return 前让对象进入明确的“无 render target”状态。

### C3 texture 文件缺失后继续对 null image 做 Vulkan 操作

- commit：`7ba65a6`
- 文件：`catvulkan/cat/simplevulkan.cpp:250` `_createTextureImageFromFile`，`catvulkan/cat/simplevulkan.cpp:1550` `svkCreateTexture(filename)`
- 风险：`fopen` 失败后 `_createTextureImageFromFile` 只 `return`，但上层继续对 `_svkTexture.image == VK_NULL_HANDLE` 调 `_setImageLayout` 和 `_createSamplerAndImageView`。资源缺失从“fopen 崩”变成 Vulkan null image / null imageView 崩。
- 建议：让 `_createTextureImageFromFile` 返回 `bool`，失败时 `svkCreateTexture` 直接返回无效纹理并由 `VulkanRender::createTexture/Env::getTextureFile` fail closed；或创建 1x1 fallback texture，禁止继续走 layout transition。

### C4 场景资源析构早于 VulkanRender waitIdle，仍有 in-flight UAF

- commit：`e7010b8` / `99ac00f`
- 文件：`catvulkan/cat/vulkanRender.cpp:455` `releaseTexture/releaseVertexBuffer/releaseIndexBuffer`，`testCat/client.cpp:135` `Client::~Client`
- 风险：`~VulkanRender` 增加 `waitIdle()` 只保护 render 自身析构；但 `Client::~Client` 先 delete scene / terrain / env，期间会释放 buffer / texture / shader，而 `m_render.release()` 还是空函数。若上一帧 GPU 仍 in-flight，资源会早于 `~VulkanRender::waitIdle()` 被销毁，仍有 UAF / device lost 风险。
- 建议：`VulkanRender::release()` 至少 `waitIdle()`，并在 `Client::~Client` 最前面调用；更好的方案是 resource release 走 fence-retire 队列，按 frame 完成后延迟销毁。

## Warning

### W1 dynamic UBO range 与实际 buffer size 不匹配

- commit：`7353b3b`
- 文件：`catvulkan/cat/vulkanRender.cpp:796` `_fillUniformData`，`catvulkan/cat/vulkanRender.cpp:918` `_fillDynamicOffsets`
- 风险：dynamic UBO descriptor range 固定为 `maxUniformBufferRange - 1`，而 per-frame uniform buffer 预算按实际写入大小约 4.25MB。靠近 `MAX_DRAW_PER_FRAME` 时，`dynamicOffset + descriptor.range` 可能越过 buffer size，触发 validation 或 driver 越界。并且 `_fillDynamicOffsets` 预算失败只 `assert + return 0`，release 下 caller 继续 bind dynamic UBO，`dynamicOffsetCount` 不匹配 layout。
- 建议：descriptor range 使用实际最大 per-binding 范围，如 MVP 64B、joint `sizeof(matrix) * MAX_JOINT_PER_OBJECT`；预算失败后 `draw2` 必须 fail closed，不再 bind / draw。

### W2 indexOffset 只 assert，Release 仍可能巨大 VkDeviceSize

- commit：`7353b3b`
- 文件：`catvulkan/cat/vulkanRender.cpp:1249` `draw2`
- 风险：`indexOffset >= 0` 只用 C `assert`，Release 下负数仍会 cast 到 `VkDeviceSize`，变成巨大 offset，可能直接 GPU 越界 / device lost。
- 建议：改为运行时守卫：`if (indexOffset < 0) { log/assert; return; }`，并校验 offset 按 index type 对齐且不超过 index buffer size。

### W3 VK_SUBOPTIMAL_KHR 被当失败重试并可能 abort

- commit：`7ba65a6`
- 文件：`catvulkan/cat/simplevulkan.cpp:2203` `svkAcquireNextImage`
- 风险：`VK_SUBOPTIMAL_KHR` 是可继续使用的成功态，但当前 `err != VK_SUCCESS` 会 callback、重试，连续 suboptimal 会 5 次后 `abort()`。窗口 resize / Android surface 状态变化时可能把可渲染状态变成进程退出。
- 建议：把 `VK_SUCCESS` 和 `VK_SUBOPTIMAL_KHR` 都视为 acquire success；suboptimal 只标记后续 recreate，不应在 acquire 内重试 / abort。

## Suggestion

### S1 单点 pick 仍使用全屏 render target

- commit：`7353b3b`
- 文件：`catvulkan/cat/vulkanRender.cpp:116` pick buffer / pick render target
- 风险：CPU pick buffer 已降到 4 字节，但 pick render target 仍是全 surface color + depth。移动端单点拾取仍要全屏 render target load / store，1080p 下有带宽浪费。
- 建议：pick pass 只在点击帧触发，并考虑 1x1 / 小尺寸 offscreen RT、scissor 限制或局部拾取路径。
