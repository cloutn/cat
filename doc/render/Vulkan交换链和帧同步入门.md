# Vulkan 交换链和帧同步入门

这篇文档是为了重新捡起 swapchain、acquire、present、semaphore、fence 这几件事的上下文。重点不是按 Vulkan spec 的顺序背 API，而是先把“窗口后面到底有几张图，CPU / GPU / 显示系统各自在什么时候能碰它们”讲清楚，然后再回到 cat 当前代码里的 C6 问题。

## 从 OpenGL 的 SwapBuffers 说起

OpenGL 里通常是这样：

```text
你画东西 -> 画到 back buffer -> SwapBuffers() -> 显示器以后拿它显示
```

代码上一般只需要关心：

```cpp
glClear();
draw();
SwapBuffers();
```

至于下面这些问题：

```text
back buffer 现在能不能画？
上一帧 GPU 画完没有？
显示器是不是还在读这张图？
窗口 resize 后 back buffer 有没有重建？
```

大多是 driver / WGL / EGL 帮你藏起来了。

Vulkan 不藏。Vulkan 说：你要显示窗口，就必须自己管理一组“可显示图片”，自己拿一张，自己画，自己交给显示系统。

这组“可显示图片”就是 swapchain。

## Swapchain 是什么

可以把 swapchain 想成窗口后面的一排图片：

```text
swapchain:
	image 0
	image 1
	image 2
```

这些图片不是普通贴图，它们是“最终可以被窗口显示”的图片。

每帧 Vulkan 的基本流程是：

```text
1. acquire：向窗口系统要一张当前可以画的 image
2. render ：往这张 image 里画
3. present：把这张 image 交给窗口系统显示
```

对应 Vulkan API 大概是：

```text
vkAcquireNextImageKHR -> vkQueueSubmit -> vkQueuePresentKHR
```

注意第一步：你不能自己拍脑袋说“我这帧画 image 1”。你要问窗口系统：

```text
现在哪张 swapchain image 可以给我画？
```

窗口系统返回一个 `imageIndex`。

比如它可能返回：

```text
这帧你可以画 image 2
```

那这一帧就应该使用 image 2 对应的 framebuffer / command buffer / fence 等相关资源。

## 为什么要同步

这里有三个角色：

```text
CPU            ：录制命令，提交命令
GPU            ：执行绘制命令
Present Engine ：窗口 / 显示系统，负责拿 swapchain image 去显示
```

它们是异步的。CPU 调用了函数，不代表 GPU 立刻做完；GPU 做完，也不代表显示系统已经不用那张图了。

所以需要同步对象。

Vulkan 里这条链通常是：

```text
vkAcquireNextImageKHR
	signal imageAcquireSemaphore
		↓
vkQueueSubmit
	wait imageAcquireSemaphore
	执行绘制命令
	signal drawCompleteSemaphore
		↓
vkQueuePresentKHR
	wait drawCompleteSemaphore
	显示 image
```

翻译成人话：

```text
imageAcquireSemaphore：窗口系统说“这张图现在可以给你画了”
drawCompleteSemaphore：GPU 说“我画完了，可以拿去显示了”
```

这两个 semaphore 必须一一对应。

假设 acquire 时传进去的是：

```text
frames[2].imageAcquireSemaphore
```

那 submit 时就必须 wait：

```text
frames[2].imageAcquireSemaphore
```

如果 submit 去 wait 了另一个：

```text
frames[0].imageAcquireSemaphore
```

那 GPU 就会等一个从来没人 signal 的 semaphore。于是卡死。

## Semaphore 和 Fence 的精确定义

一句话简化版是：

```text
semaphore 用来给异步执行体之间排顺序
fence     用来让 CPU 观察 GPU / queue 的工作是否完成
```

这里要特别注意：“异步执行体”不只包括 GPU graphics queue，也包括 WSI / present engine 这种窗口显示系统相关的东西。

所以把 semaphore 简单说成“GPU 之间通信”不够准确。对 swapchain 来说，更好的说法是：

```text
semaphore 负责 GPU queue 和 WSI / present engine 之间的异步顺序；
fence     负责 CPU 等待 GPU queue 完成。
```

### imageAcquireSemaphore

`vkAcquireNextImageKHR` 这一步有两层含义：

```text
1. CPU 调用函数，向 WSI 要一张 swapchain image
2. WSI 通过 semaphore / fence 通知“这张 image 真正可以被后续 GPU 使用”
```

cat 当前代码里是：

```cpp
vkAcquireNextImageKHR(
	device.device,
	swapchain.swapchain,
	UINT64_MAX,
	frames[frame].imageAcquireSemaphore,
	VK_NULL_HANDLE,
	&nextFrame);
```

因为 timeout 是 `UINT64_MAX`，所以这个 CPU 调用本身确实可能阻塞。比如三张 swapchain image 都还在显示队列里，CPU 就可能卡在 acquire 里等一张变得可用。

但重点是：即使 CPU 拿到了 image index，后面的 GPU submit 仍然要 wait acquire semaphore。

它不是：

```text
CPU 等完了，所以 GPU 直接画
```

而是：

```text
CPU 拿到 imageIndex
GPU submit 等 imageAcquireSemaphore
WSI signal imageAcquireSemaphore
GPU 开始往这张 image 画
```

所以 `imageAcquireSemaphore` 更像：

```text
Present Engine / WSI -> GPU graphics queue
```

不是 CPU -> GPU，也不是 GPU -> CPU。

### drawCompleteSemaphore

`drawCompleteSemaphore` 也不是“显示器这个硬件本体直接等待 GPU”。更准确地说，是 present 操作等待 GPU signal 的 semaphore。

流程是：

```text
vkQueueSubmit:
	GPU 执行绘制
	signal drawCompleteSemaphore

vkQueuePresentKHR:
	wait drawCompleteSemaphore
	然后 WSI / present engine 才能拿这张 image 去显示
```

所以 `drawCompleteSemaphore` 是：

```text
GPU graphics queue -> Present Engine / WSI
```

它也不是纯 GPU -> GPU，而是 graphics queue 到 present queue / present engine 的同步。

### fence

fence 是 CPU 能等的：

```cpp
vkWaitForFences(...);
```

它表达的是：

```text
CPU：GPU，你上次提交的那批活干完了吗？
```

cat 代码里用 fence 是为了复用 per-frame 资源：

```text
commandBuffer
framebuffer
uniform buffer offset
descriptor / allocator 状态
```

如果 GPU 还在用这些资源，CPU 不能重置它们。

可以这样记：

```text
semaphore:
	给 GPU queue / present engine 这类异步执行体排顺序
	CPU 通常不直接 wait 它

fence:
	给 CPU 等 GPU / queue 完成用
```

更准确的一张表：

```text
imageAcquireSemaphore:
	signal 方：WSI / present engine
	wait 方  ：GPU graphics queue
	目的    ：别在窗口系统还没释放 image 时就开始画

drawCompleteSemaphore:
	signal 方：GPU graphics queue
	wait 方  ：WSI / present engine / present queue
	目的    ：别在 GPU 还没画完时就拿去显示

fence:
	signal 方：GPU queue submit 完成
	wait 方  ：CPU
	目的    ：CPU 确认这套 frame 资源可以复用了
```

## cat 里的 svkFrame

cat 当前的 `svkFrame` 混合了两类东西。

每个 `svkFrame` 里有：

```text
imageView / framebuffer      跟 swapchain image index 绑定
commandBuffer / fence        跟本帧提交资源有关
imageAcquireSemaphore        acquire 用
drawCompleteSemaphore        present 用
```

也就是说：

```text
frames[0] 对应 swapchain image 0 的 framebuffer
frames[1] 对应 swapchain image 1 的 framebuffer
frames[2] 对应 swapchain image 2 的 framebuffer
```

这套写法可以跑，但有一个地方比较绕：

```text
vkAcquireNextImageKHR 在调用前，你还不知道它会返回哪个 image index。
```

可是 acquire 需要你提前传一个 semaphore：

```cpp
vkAcquireNextImageKHR(..., someSemaphore, ..., &nextFrame);
```

问题来了：

```text
我还不知道 nextFrame 是几，怎么传 frames[nextFrame].imageAcquireSemaphore？
```

所以现在代码用的是“上一轮的 m_frameIndex”来挑 semaphore。

当前 `beginDraw()` 的形状是：

```cpp
m_prevFrameIndex = m_frameIndex;
m_frameIndex = svkAcquireNextImage(m_device, m_swapchain, m_frames, m_frameIndex, this, presentCallback);
```

这里 `m_prevFrameIndex` 这个名字有点误导。它不是单纯的“上一帧画的 image”。在这个流程里，它真正表示：

```text
本次 acquire 使用的是哪个 imageAcquireSemaphore
```

也就是：

```text
m_prevFrameIndex = acquireSemaphoreIndex
```

然后 `svkAcquireNextImage()` 里 acquire 使用它：

```cpp
frames[frame].imageAcquireSemaphore
```

再到 submit 时等它：

```cpp
&m_frames[m_prevFrameIndex].imageAcquireSemaphore
```

正常情况是对的。

举例：

```text
进入 beginDraw 前：
m_frameIndex = 2

beginDraw:
m_prevFrameIndex = 2

svkAcquireNextImage(... frame = 2 ...)
	acquire signal frames[2].imageAcquireSemaphore
	返回 nextFrame = 1

m_frameIndex = 1

endScenePass:
	渲染 frames[1].commandBuffer / framebuffer
	wait   frames[2].imageAcquireSemaphore
	signal frames[1].drawCompleteSemaphore
```

这个虽然看起来怪，但逻辑成立：

```text
acquire signal 的是 frames[2].imageAcquireSemaphore
submit  wait   的也是 frames[2].imageAcquireSemaphore
```

## Resize / recreate swapchain 是什么

窗口 resize、最小化恢复、surface lost 等情况，会导致旧 swapchain 不再适合当前窗口。

这时 Vulkan 通常会从 acquire 或 present 返回：

```text
VK_ERROR_OUT_OF_DATE_KHR
VK_ERROR_SURFACE_LOST_KHR
VK_SUBOPTIMAL_KHR
```

cat 当前 callback 里做了这件事：

```cpp
if (err == VK_ERROR_OUT_OF_DATE_KHR)
{
	render->waitIdle();
	render->recreateSwapchain();
}
```

`recreateSwapchain()` 最后会销毁旧的 main render target，然后重新创建 swapchain、framebuffer、semaphore、fence 等资源。

关键是，重新创建 main render target 时会重置 frame 状态：

```cpp
m_frameIndex     = 0;
m_prevFrameIndex = 0;
```

也就是说，一旦 recreate，当前 frame 状态被重置了。

## C6 问题：acquire 失败后 semaphore 错位

现在 `svkAcquireNextImage()` 内部是这个结构：

```text
do
{
	vkAcquireNextImageKHR(... frames[frame].imageAcquireSemaphore ...)

	if failed:
		callback()
		retry
}
while failed
```

问题是：`frame` 是函数参数，是调用 `svkAcquireNextImage()` 时传进去的旧值。

假设一开始：

```text
m_frameIndex = 2
```

进入 `beginDraw()`：

```text
m_prevFrameIndex = 2
svkAcquireNextImage(... frame = 2 ...)
```

第一次 acquire 失败：

```text
vkAcquireNextImageKHR 返回 VK_ERROR_OUT_OF_DATE_KHR
```

然后 callback 里 recreate swapchain：

```text
recreateSwapchain()
	destroy old frames
	create new frames
	m_frameIndex     = 0
	m_prevFrameIndex = 0
```

注意，现在成员变量已经被 reset 成 0 了。

但是 `svkAcquireNextImage()` 还没退出，它的局部参数 `frame` 仍然是旧值：

```text
frame = 2
```

于是它 retry：

```text
vkAcquireNextImageKHR(... frames[2].imageAcquireSemaphore ...)
```

如果这次成功：

```text
signal frames[2].imageAcquireSemaphore
返回 nextFrame = 1
```

回到 `beginDraw()`：

```text
m_frameIndex = 1
```

但是 `m_prevFrameIndex` 呢？

它已经在 recreate 里被改成 0 了。`beginDraw()` 不会再重新设置一次。

于是 submit 时：

```text
wait frames[m_prevFrameIndex].imageAcquireSemaphore
= wait frames[0].imageAcquireSemaphore
```

最终变成：

```text
acquire signal: frames[2].imageAcquireSemaphore
submit wait   : frames[0].imageAcquireSemaphore
```

这就是 signal / wait semaphore 错位。

如果 `frames[0].imageAcquireSemaphore` 没被 signal，GPU submit 就会一直等。后面的 fence 也不会 signal。CPU 下一帧再等 fence，就可能表现为死等。

## 为什么不应该在 svkAcquireNextImage 内部 callback 后继续

因为 `svkAcquireNextImage()` 是个底层 helper，但 callback 里做的是高层状态重建：

```text
销毁 frames
重建 frames
重置 m_frameIndex
重置 m_prevFrameIndex
重建 framebuffer / render target
```

helper 函数手里还握着旧的局部 `frame`，它不知道外面的状态已经被 callback 改过了。

所以更稳的原则是：

```text
svkAcquireNextImage 只负责 acquire 一次，并把结果告诉外层。
外层 beginDraw 负责判断失败、recreate、重新开始当前帧。
```

也就是：

```text
beginDraw:
	acquireSemaphoreIndex = m_frameIndex

	result = acquire(acquireSemaphoreIndex, &imageIndex)

	if result == OUT_OF_DATE:
		recreateSwapchain()
		从新的 m_frameIndex 重新 acquire

	if success:
		m_prevFrameIndex = acquireSemaphoreIndex
		m_frameIndex = imageIndex
```

这样可以保证：

```text
m_prevFrameIndex 永远记录“本次成功 acquire 实际使用的 semaphore index”
```

而不是中途被 recreate 改掉。

## 后续更清晰的帧模型

第一步止血之后，更推荐从结构上把两个概念拆开：

```text
frame-in-flight index：当前使用第几个 CPU / GPU 帧上下文
swapchain image index：vkAcquireNextImageKHR 返回的窗口图片编号
```

现在 cat 的绕法，是因为这两个东西混在了 `m_frameIndex` / `svkFrame` 里。

行业里比较常见的写法是：

```text
acquire semaphore：用 frame-in-flight index 选
framebuffer / image：用 acquire 返回的 imageIndex 选
present semaphore：最好用 imageIndex 选
```

也就是：

```text
frameIndex = 0, 1, 0, 1...              // 自己轮转，acquire 前就知道
imageIndex = vkAcquireNextImageKHR(...) // WSI 返回，acquire 后才知道
```

所以那个绕人的问题：

```text
我还不知道 imageIndex，怎么传 frames[imageIndex].imageAcquireSemaphore？
```

更成熟的答案是：

```text
不要传 frames[imageIndex].imageAcquireSemaphore。
acquire semaphore 本来就不该跟 imageIndex 绑死。
它应该来自 frame-in-flight context。
```

可以把资源拆成两组。

第一组是 `FrameContext`，按 frame-in-flight 数量分配，比如 2 或 3 个：

```cpp
struct svkFrameContext
{
	VkCommandBuffer	commandBuffer;
	VkFence			fence;
	VkSemaphore		imageAcquireSemaphore;
};
```

它负责：

```text
CPU / GPU 帧上下文
命令录制
CPU 等 GPU 完成
acquire semaphore
```

第二组是 `SwapchainFrame`，按 swapchain image 数量分配：

```cpp
struct svkSwapchainFrame
{
	VkImageView		imageView;
	VkFramebuffer	framebuffer;
	VkSemaphore		drawCompleteSemaphore;
};
```

它负责：

```text
swapchain image 对应的 framebuffer
present 要等待的 draw complete semaphore
```

一帧流程会变成：

```text
frameContext = m_frameContexts[m_frameContextIndex]

wait frameContext.fence
reset frameContext.fence

vkAcquireNextImageKHR(
	swapchain,
	frameContext.imageAcquireSemaphore,
	&imageIndex)

swapchainFrame = m_swapchainFrames[imageIndex]

record commandBuffer:
	使用 swapchainFrame.framebuffer

submit:
	wait   frameContext.imageAcquireSemaphore
	signal swapchainFrame.drawCompleteSemaphore
	fence  frameContext.fence

present:
	imageIndex
	wait swapchainFrame.drawCompleteSemaphore

m_frameContextIndex = (m_frameContextIndex + 1) % MAX_FRAMES_IN_FLIGHT
```

这里还有一个更深的同步点：`vkQueuePresentKHR` 等待的 semaphore 很容易复用错。因为 present 不像 queue submit 那样能直接 signal fence，所以不能只靠 submit fence 判断“present 已经不用这个 semaphore 了”。

Khronos Guide 推荐的稳妥做法是：

```text
submit finished / render finished semaphore 按 swapchain image 数量分配，
并用 acquired image index 索引。
```

所以最终更清晰的形态是：

```text
FrameContext[frameIndex]:
	fence
	commandBuffer
	imageAcquireSemaphore

SwapchainFrame[imageIndex]:
	imageView
	framebuffer
	drawCompleteSemaphore
```

这样代码以后读起来就是：

```text
frameContextIndex：这次提交用哪套 CPU / GPU 临时资源
imageIndex       ：这次画哪张窗口图片
```

这两个索引就不会再互相冒充。

## 最短总结

OpenGL 把“拿 back buffer、等它可用、画完再显示”都藏了。Vulkan 要你显式写出来。

cat 这段代码正常帧没问题，问题是 acquire 失败后，callback 在 `svkAcquireNextImage()` 内部重建了 swapchain，并重置了 `m_frameIndex / m_prevFrameIndex`。但 `svkAcquireNextImage()` 继续用进入函数时的旧 `frame` 去 retry acquire，导致 acquire signal 的 semaphore 和 submit wait 的 semaphore 不是同一个。

这个问题的本质是：

```text
swapchain recreate 是会改变帧状态的大动作，
不应该藏在 acquire helper 内部继续跑；
应该返回给 beginDraw，
让当前帧从新的状态重新开始。
```
