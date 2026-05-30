# 子系统 — catvulkan（Vulkan 渲染后端）

> 上级：[架构总览](./架构总览.md)

`catvulkan` 是 `cat::IRender` 的 Vulkan 实现，同时维护引擎所需的 Vulkan 资源缓存（管线、描述符集、命令缓冲）。它隔在 `cat` 与原生 Vulkan API 之间：上层只看 `IRender`，本模块独占 `VkXxx` 句柄与生命周期。

## 1. 模块组成

| 文件 | 角色 |
|------|------|
| `cat/simplevulkan.h/.cpp` | Vulkan API 的薄封装（svk* 前缀）：instance、surface、device、swapchain、command、descriptor、pipeline、render pass、buffer、image、frame、fence、semaphore。是本模块的"L0"。 |
| `cat/vulkanRender.h/.cpp` | `VulkanRender : public IRender`，提供 `draw2 / beginDraw / endDraw / beginScenePass / beginPickPass` 等高层调用，组合 L0 与各类缓存器。 |
| `cat/commandAllocator.h/.cpp` | 单 frame 内 `VkCommandBuffer` 的池化分配器（线性 alloc，整体 reset）。 |
| `cat/descriptorAllocator.h/.cpp` | `VkDescriptorSet` 的分页式分配器（每页 64 set + 一个 pool），支持单 set 释放与重新利用。 |
| `cat/pipelineKey.h/.cpp` | 管线缓存的键：vertex attr + shader + topology + render pass。 |
| `cat/DescriptorDataKey.h/.cpp` | 描述符集缓存的键：根据 bind 列表 + 实际 buffer/texture 句柄 + dynamic offset 计算 hash。 |
| `cat/deviceInfo.h` | 设备能力快照（`DeviceInfo` + `DeviceLimits`），由 `_fillDeviceInfo` 在 init 时填充，编辑器 UI 直接消费。 |
| `cat/imgui_impl_vulkan.h/.cpp` | imgui 官方的 Vulkan 后端（按需小改），由 `VulkanRender::initIMGUI / drawIMGUI` 调用。 |

## 2. simplevulkan（svk* 薄封装）

`simplevulkan.h` 定义了一组紧凑结构体把相关 Vulkan 对象打包：

- `svkDevice`：`VkDevice` + 物理设备 + 队列家族 + 默认 commandPool + `shaderc_compiler_t`。
- `svkSurface / svkSwapchain / svkFrame`：交换链与每帧资源（imageView / framebuffer / commandBuffer / fence / 两个 semaphore）。
- `svkImage / svkTexture / svkBuffer / svkPipeline / svkShaderProgram`：分别对一类 GPU 资源做"句柄 + 内存 + 配套元数据"打包。
- `svkDescriptorData`：单一 binding 的实际数据载体；union 区分 buffer / texture，最多 64 个 element（数组 binding 支持）。
- `svkShaderProgram`：vertex/tcs/tes/geo/frag/comp 六个 `VkShaderModule` + 反射出的 `descriptorSetLayoutBinds` 和 `pushConstRanges`。

提供的 svk* 自由函数按生命周期 / 资源类型分组（见 `simplevulkan.h` 末尾函数声明），命名一致：`svkCreate* / svkDestroy* / svkCmd*`。`svkCreateShaderProgramFromFile` 是核心：内部用 `shaderc` 把 GLSL 编译为 SPIR-V，再用 `spirv-cross` 反射出 binding/range。

## 3. VulkanRender —— IRender 的实现

### 3.1 生命周期

- `init(hInstance, hwnd)`：建 instance → device → surface → swapchain → 主 RenderPass + depth → frames → pick RenderTarget → 各类缓存表（pipelines / descriptorAllocators / descriptorSetCache）→ shaderc 编译器 → uniform buffer。
- `recreateSurface() / recreateSwapchain()`：窗口大小变化时调用；保留 device，重建 surface + swapchain + frames + 主 RenderTarget。
- `release()`：反向销毁全部资源。

### 3.2 每帧驱动

`VulkanRender` 把"一帧"拆为：

1. `beginDraw()`：`svkAcquireNextImage` 拿到 swapchain image → `m_frameIndex` → 重置当前 frame 的 `CommandAllocator` 与 `descriptorSetCache`。
2. **可选** `beginPickPass / endPickPass(x,y)`：渲染到 `m_pickRenderTarget`（独立 RenderPass、独立 CommandBuffer、独立 fence），结束后 `svkCopyImageToData` 把目标像素带回 CPU，反查 `Primitive*`。
3. `beginScenePass / endScenePass`：开始主 RenderPass，业务侧每个 `Primitive::draw` 会回调 `IRender::draw2`。
4. `drawIMGUI(draw_data)`：在 ImGui pass 中渲染 UI。
5. `swap()`：`svkQueueSubmitFrame` + `svkPresent`。

### 3.3 draw2 主路径

`draw2(...)` 是 `cat` 唯一进入的入口，做四件事：

1. `_prepareDescriptorSetAndFillData(...)`：
   - 用 `_fillUniformData` 把 mvp + texture + joint matrices 装入 `svkDescriptorData[]`；
   - 用 `_fillDynamicOffsets` 在 frame uniform buffer 中开辟对齐的子区域（按 `minUniformBufferOffsetAlignment` 对齐）；
   - 用 `DescriptorDataKey` 查 `m_descriptorSetCache`，命中复用、未命中走 `_prepareDescriptorSet` 创建并 `svkUpdateDescriptorSet`。
2. `_preparePipeline(...)`：
   - 用 `PipelineKey(vertexAttr, shader, topology, renderPass)` 查 `m_pipelines`，未命中调 `svkCreatePipelineEx`（顶点输入布局由当前 `VertexAttr` 数组现场拼出）。
3. `_fillPushConst(...)`：从 `svkShaderProgram::pushConstRanges` 找到目标 stage，把 `pushConstBuffer` 拷入。
4. `vkCmdBindVertexBuffers / vkCmdBindIndexBuffer / vkCmdBindDescriptorSets / vkCmdBindPipeline / vkCmdDrawIndexed` —— 最终命令录制。

### 3.4 内部缓存

| 缓存 | 类型 | 键 | 容量 |
|------|------|----|------|
| `m_pipelines` | `hash_table<PipelineKey, svkPipeline*>` | vertex attr + shader + topology + render pass | `MAX_PIPELINE_COUNT = 1024` |
| `m_descriptorAllocators` | `hash_table<int, DescriptorAllocator*>` | uniform bind 列表的 hash | `MAX_DESCRITOR_ALLOCATOR_COUNT = 1024` |
| `m_descriptorSetCache` | `hash_table<DescriptorDataKey, DescriptorSet>` | binding + 实际 buffer/texture 句柄 + dynamic offsets | `MAX_DESCRIPTOR_SET_CACHE_SIZE = 1024` |

descriptorSetCache 在每帧 `beginDraw` 重置；`pipelines` 与 `descriptorAllocators` 跨帧保留。

## 4. 关键辅助类

### 4.1 CommandAllocator

`CommandAllocator`（`commandAllocator.h`）：每个 frame 一个，本质是「自管的 `VkCommandPool` + 顺序 alloc 的 256 个 `VkCommandBuffer`」。`reset(device)` 整体重置 pool，alloc 计数清零；不支持单条释放。适合「一帧 N 个 secondary CB」这种用法。

### 4.2 DescriptorAllocator / DescriptorPage

`DescriptorAllocator`（`descriptorAllocator.h`）支持单 set 释放与重用：

- 每个 `DescriptorPage` = 一个 `VkDescriptorPool` + 64 个预创建的 `VkDescriptorSet` + `scl::stack<int>` 空闲索引栈。
- `alloc()` 优先从有空闲索引的 page 出，所有 page 都满则增 page（上限 `MAX_PAGE_COUNT = 256`）。
- `free(set)` 把 set 推回对应 page 的 freeIndices，无须 `vkFreeDescriptorSets`（pool 必须以 `FREE_DESCRIPTOR_SET_BIT` 创建，由 `svkCreateDescriptorCreatorEx` 控制）。
- 一个 allocator 对应一个 layout；layout 变了必须新建 allocator —— 这与 `m_descriptorAllocators` 按 bind hash 索引契合。

### 4.3 PipelineKey

四元组 + hash：避免在每次 `draw2` 时重建管线。需要注意 `vertexAttrs / shader` 是指针比较 —— 上层必须保证「同语义同指针」（这正是 `ShaderCache` 与 cat 层 vertex attr 单例化的目的；详见 `pipelineKey.h` 中的 TODO 注释）。

### 4.4 DescriptorDataKey

把"哪些 binding + 各 binding 的实际句柄 + dynamic offset"展开到 `m_dataHandles[1024]`，整体 hash。命中率高时，单帧可大幅减少 `vkUpdateDescriptorSets` 调用。注意：

- 当前实现假设 binding 数 + 数据数 ≤ 1024；
- `m_descriptorSetCache` 每帧清空，因此 dynamic offset 跨帧不会撞键。

### 4.5 DeviceInfo

`DeviceInfo` 在 `init` 阶段一次性把 `VkPhysicalDeviceProperties / Limits` 拷贝到引擎自己的 POD（`deviceInfo.h`），后续 UI、布局对齐计算（`minUniformBufferOffsetAlignment` 等）都从这里取，**不再回 Vulkan API 查询**。

## 5. 与上层的契约

- `cat::IRender` 中的 `void*` 在本后端的真实类型：
  - VB / IB → `svkBuffer*`（指针，便于 release 时直接拿 memory）
  - Texture → `svkTexture*`
  - Shader → `svkShaderProgram*`
- `draw2` 的 `pushConstBuffer` 由调用方负责对齐与裁剪到 shader 的 push constant range 大小；超出 range 的部分会被 `_fillPushConst` 截断。
- `getDeviceWidth/Height` 返回 surface 大小（与 swapchain 一致），编辑器据此布局。

## 6. ImGui 后端

`imgui_impl_vulkan.cpp` 是 ImGui 官方实现的轻微改造版（与上游主要差异：用本引擎的 `IFileProvider` 读字体、与 `VulkanRender` 共享 RenderPass）。`VulkanRender::initIMGUI` 负责创建独立的 `m_IMGUIDescriptorPool`，`drawIMGUI(draw_data)` 在 endScenePass 之后调用即可把 UI 叠加到主 RenderPass。
