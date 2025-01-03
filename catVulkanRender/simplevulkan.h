//////////////////////////////
// 2021.02.14 caolei
//////////////////////////////
#pragma once

#include <vulkan/vulkan_core.h>
#include <shaderc/shaderc.h>

//TODO 很多函数的参数应该添加 const 修饰符

struct svkDevice
{
	VkDevice							device;
	VkPhysicalDevice					gpu;
	VkPhysicalDeviceProperties			gpuProperties;
	VkPhysicalDeviceMemoryProperties	memoryProperties;
	VkQueue								queue;
	unsigned int						queueFamilyCount;
	unsigned int						queueFamilyIndex;
	VkCommandPool						commandPool;		//! obsolete, manager command pool by yourself, for example, class CommandAllocator
	//VkFormatProperties					formatProperties;	// VK_FORMAT_R8G8B8A8_UNORM  properties
	shaderc_compiler_t					shaderCompiler;
};

struct svkSurface
{
	VkSurfaceKHR						surface;
	int									width;
	int									height;
	VkSurfaceCapabilitiesKHR			capabilities;
};

struct svkImage
{
	//depth
	VkFormat							format;
	VkImage								image;
	VkDeviceMemory						memory;
	VkImageView							imageView;
};

struct svkSwapchain
{
	static const int					MAX_IMAGE_COUNT			= 4;

	VkSwapchainKHR						swapchain;
	VkFormat							format;
	VkColorSpaceKHR						colorSpace;
	unsigned int						imageCount;
};

struct svkFrame
{
	VkImageView							imageView;
	VkFramebuffer						framebuffer;
	VkCommandBuffer						commandBuffer;
	VkFence								fence;
	VkSemaphore							imageAcquireSemaphore;
	VkSemaphore							drawCompleteSemaphore;
};

struct svkTexture
{
	VkSampler							sampler;
	VkImage								image;
	VkBuffer							buffer;
	VkImageLayout						imageLayout;
	VkDeviceMemory						memory;
	VkImageView							view;
	int32_t								width;
	int32_t								height;
};

struct svkBuffer
{
	VkBuffer							buffer;
	VkDeviceMemory						memory;
};

struct svkPipeline
{
	VkPipeline							pipeline;
	VkPipelineLayout					layout;
	VkPipelineCache						cache;
};

struct svkDescriptorCreator
{
	VkDescriptorPool					pool;
	VkDescriptorSetLayout				layout;
};

struct svkDescriptorData
{
	struct BufferInfo
	{
		VkBuffer		buffer;
		int				bufferSize;
	};
	struct TextureInfo
	{
		VkSampler		sampler;
		VkImageView		imageView;
	};

	union Info 
	{
		BufferInfo		buffer;
		TextureInfo		texture;
	};

	Info				data[64];	// uniform 数据。单个 binding 点支持64个 buffer 或者 texture
	int					binding;	// 对应 shader 的 DescriptorSetLayout 中的 binding 值。
	int					dataCount;	// data 数组中有效数据的个数
};

struct svkShaderProgram
{
	VkShaderModule					vert;
	VkShaderModule					tcs;
	VkShaderModule					tes;
	VkShaderModule					geo;
	VkShaderModule					frag;
	VkShaderModule					comp;

	// uniform bindings
	VkDescriptorSetLayoutBinding*	descriptorSetLayoutBinds;
	int								descriptorSetLayoutBindCount;

	VkPushConstantRange*			pushConstRanges;
	int								pushConstRangeCount;
};

// instance and device
VkInstance				svkCreateInstance				(bool enableValidationLayer = false);
void					svkDestroyInstance				(VkInstance);
svkDevice				svkCreateDevice					(VkInstance);
void					svkDestroyDevice				(svkDevice& device);
void					svkDeviceWaitIdle				(svkDevice& device);

// surface
svkSurface				svkCreateSurface				(VkInstance inst, svkDevice& device, void* hinstance, void* hwnd);
void					svkDestroySurface				(VkInstance, svkDevice&, svkSurface&);
void					svkRefreshSurfaceSize			(svkDevice&, svkSurface&);

// command buffer
VkCommandPool			svkCreateCommandPool			(svkDevice& device, bool needResetIndividual);
VkCommandBuffer			svkAllocCommandBuffer			(svkDevice& device);
void					svkAllocCommandBuffer			(svkDevice& device, VkCommandPool pool, bool isPrimary, int count, VkCommandBuffer* output);
void					svkFreeCommandBuffer			(svkDevice& device, VkCommandBuffer commandBuffer);
void					svkBeginCommandBuffer			(VkCommandBuffer, bool oneTime = false);
void					svkEndCommandBufferAndSubmit	(svkDevice&, VkCommandBuffer);
void					svkBeginSecondaryCommandBuffer	(VkCommandBuffer, const VkRenderPass renderPass, const VkFramebuffer framebuffer);
void					svkResetCommandBuffer			(VkCommandBuffer cb);
VkResult				svkEndCommandBuffer				(VkCommandBuffer cb);

// texture
//		TODO png支持使用 gfx 代码接入引擎
//		TODO 当文件的 pitch 和 vulkan createImage 的 pitch 不一致的时候，需要手动重新拷贝一次，而不能直接使用 map memory 返回的指针来创建。
svkTexture				svkCreateTexture				(svkDevice&, const char* const filename, VkCommandBuffer commandBuffer);
svkTexture				svkCreateTexture				(svkDevice& device, const int width, const int height, VkCommandBuffer outCommandBuffer);
//		TODO 没有检查目标 texutrre 的大小和是否可以写入
void					svkCopyTexture					(svkDevice& device, svkTexture& texture, const void* const data, const int sizeofData);
void					svkDestroyTexture				(svkDevice&, svkTexture&);
void					svkCopyImageToData				(svkDevice& device, svkImage& image, void* const data, const int dataCapacity, int* outCopiedByteCount);

//shader
svkShaderProgram		svkCreateShaderProgramFromCode	(svkDevice& device, const char* const vertCode, const char* const tcsCode, const char* const tesCode, const char* const geoCode, const char* const fragCode, const char* const compCode);
svkShaderProgram		svkCreateShaderProgramFromFile	(svkDevice& device, const char* const vertFilename, const char* const tcsFilename, const char* const tesFilename, const char* const geoFilename, const char* const fragFilename, const char* const compFilename);
void					svkDestroyShaderProgram			(svkDevice& device, svkShaderProgram&);

// descriptor set
VkDescriptorSetLayout	svkCreateDescriptorLayout		(svkDevice& device);
VkDescriptorSetLayout	svkCreateDescriptorLayoutEx		(svkDevice& device, const VkDescriptorSetLayoutBinding* binds, const int bindCount);
VkDescriptorPool		svkCreateDescriptorPoolEx		(svkDevice& device, const unsigned int size, const VkDescriptorSetLayoutBinding* binds, const int bindCount);
VkDescriptorPool		svkCreateDescriptorPoolEx2		(svkDevice& device, const int poolSize, const int* countPerType);
svkDescriptorCreator	svkCreateDescriptorCreatorEx	(svkDevice& device, const unsigned int poolSize, const VkDescriptorSetLayoutBinding* binds, const int bindCount);
void					svkDestroyDescriptorCreator		(svkDevice& device, svkDescriptorCreator&);
VkDescriptorSet			svkCreateDescriptorSet(
	svkDevice&				device, 
	VkDescriptorPool		pool, 
	VkDescriptorSetLayout	layout, 
	VkBuffer				uniformBuffer,
	const int				uniformCount, 
	const int				sizeofUniform, 
	const int				textureCount, 
	svkTexture*				textures);
VkDescriptorSet			svkCreateDescriptorSet(
	svkDevice&				device, 
	svkDescriptorCreator&	creator, 
	VkBuffer				uniformBuffer,
	const int				uniformCount, 
	const int				sizeofUniform, 
	const int				textureCount, 
	svkTexture*				textures);
void					svkUpdateDescriptorSet			(svkDevice& device, VkDescriptorSet descriptorSet, const VkDescriptorSetLayoutBinding* binds, const int bindCount, const svkDescriptorData* datas, const int dataCount);
void					svkDestroyDescriptorPool		(svkDevice& device, VkDescriptorPool pool);

// pipeline 
svkPipeline				svkCreatePipeline				(svkDevice& device, VkDescriptorSetLayout pipelineLayout, VkRenderPass renderPass, VkPipelineCache* cache);
svkPipeline				svkCreatePipelineEx				(svkDevice& device, VkDescriptorSetLayout pipelineLayout, VkRenderPass renderPass, VkPrimitiveTopology topology, VkPipelineVertexInputStateCreateInfo& viCreateInfo, svkShaderProgram& shaderProgram, VkPipelineCache* cache);
void					svkDestroyPipeline				(svkDevice&, svkPipeline&, bool deleteCache = true);

// record commands
void					svkCmdBeginRenderPass			(VkCommandBuffer cb, float clearColorR, float clearColorG, float clearColorB, float clearColorA, float depth, unsigned int stencil, VkRenderPass renderPass, VkFramebuffer framebuffer, uint32_t width, uint32_t height, bool useSecondaryCommandBuffer);
void					svkCmdSetViewPortCubic			(VkCommandBuffer cb, uint32_t width, uint32_t height);
void					svkCmdSetViewPortByGLParams		(VkCommandBuffer cb, const int x, const int y, const int width, int height);
void					svkCmdSetViewPortDirectly		(VkCommandBuffer cb, const int x, const int y, const int width, int height);
void					svkCmdBindVertexBuffers			(VkCommandBuffer cb, int firstBinding, void** vertexBuffers, int attrCount);
void					svkCmdSetScissor				(VkCommandBuffer cb, uint32_t width, uint32_t height);

// submit and present
typedef void			(*presentResultCallback)		(void* userData, VkResult);
int						svkAcquireNextImage				(svkDevice& device, svkSwapchain& swapchain, svkFrame* frames, const int frameIndex, void* userData, presentResultCallback callback);
void					svkQueueSubmit					(svkDevice& device, const VkCommandBuffer* commandBuffers, const int commandBufferCount, VkSemaphore* waitSemaphore, VkSemaphore* signalSemaphore, VkFence fence);
void					svkQueueSubmitFrame				(svkDevice& device, svkFrame* frames, const int frame, const int prevFrame);
void					svkPresent						(svkDevice& device, svkSwapchain& swapchain, svkFrame* frames, const int frame,  void* userData, presentResultCallback callback);
void					svkWaitQueue					(svkDevice&);

// swapchain
svkSwapchain			svkCreateSwapchain				(svkDevice& device, const svkSurface& surface, const svkSwapchain& oldSwapchain, int imageCount = 3, bool deleteOldSwapchainHandle = true);
void					svkDestroySwapchain				(svkDevice& device, svkSwapchain& swapchain, bool deleteSelfHandle = true);

// buffer
svkBuffer				svkCreateBuffer					(svkDevice& device, VkBufferUsageFlags usage, const int size);
void					svkCopyBuffer					(svkDevice& device, svkBuffer&, const void* data, const int dataSize);
void*					svkMapBuffer					(svkDevice& device, svkBuffer&);
void*					svkMapBuffer					(svkDevice& device, svkBuffer&, const int bytes); // bytes = -1 (VK_WHOLE_SIZE) means copy all available bytes
void*					svkMapMemory					(svkDevice& device, VkDeviceMemory& memory, const int bytes);
void					svkUnmapBuffer					(svkDevice& device, svkBuffer&);
void					svkUnmapMemory					(svkDevice& device, VkDeviceMemory& memory);
void					svkDestroyBuffer				(svkDevice& device, svkBuffer& buffer);
svkBuffer				svkCreateVertexBuffer			(svkDevice& device, const void* data, const int dataSize);
svkBuffer				svkCreateIndexBuffer			(svkDevice& device, const void* data, const int dataSize);
svkBuffer				svkCreateUniformBuffer			(svkDevice& device, void* data, const int dataSize);

// render pass
VkRenderPass			svkCreateRenderPass				(svkDevice& device, VkFormat format, VkFormat depthFormat, VkImageLayout colorAttachmentFinalLayout);
void					svkDestroyRenderPass			(svkDevice& device, VkRenderPass renderPass);

// frame buffer
VkFramebuffer			svkCreateFrameBuffer			(svkDevice& device, VkRenderPass renderPass, VkImageView* attachments, const int attachmentCount, const uint32_t width, const uint32_t height);
void					svkDestroyFrameBuffer			(svkDevice& device, VkFramebuffer framebuffer);

// image
svkImage				svkCreateAttachmentDepthImage	(svkDevice& device, VkFormat format, const int width, const int height);
svkImage				svkCreateAttachmentColorImage	(svkDevice& device, VkFormat format, const int width, const int height);
void					svkDestroyImage					(svkDevice& device, svkImage& image);
VkFormat				svkChooseColorFormat			(svkDevice& device, VkFormat* formats, const int formatCount);
VkFormat				svkChooseDepthFormat			(svkDevice& device, VkFormat* formats, const int formatCount);

// svk frame
int						svkCreateFrames					(svkDevice& device, svkSwapchain& swapchain, VkImageView depthImageView, VkRenderPass renderPass, const int width, const int height, svkFrame* outputFrames, const int outputFrameCapacity);
void					svkDestroyFrames				(svkDevice& device, svkFrame* frames, const int frameCount);

// fence
VkFence					svkCreateFence					(svkDevice& device, bool signaled);
void					svkWaitFence					(svkDevice& device, VkFence* fences, const int fenceCOunt);
void					svkDestroyFence					(svkDevice& device, VkFence fence);
bool					svkIsFenceSignaled				(svkDevice& device, VkFence fence);

// semaphore
VkSemaphore				svkCreateSemaphore				(svkDevice& device);
void					svkDestroySemaphore				(svkDevice& device, VkSemaphore semaphore);


