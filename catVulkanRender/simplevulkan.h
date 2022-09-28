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
	VkFormatProperties					formatProperties;	// VK_FORMAT_R8G8B8A8_UNORM  properties
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
	VkDescriptorSetLayoutBinding*	layoutBinds;
	int								layoutBindCount;
};

VkInstance				svkCreateInstance				(bool enableValidationLayer = false);
svkDevice				svkCreateDevice					(VkInstance);
void					svkDestroyDevice				(svkDevice& device);
svkSurface				svkCreateSurface				(VkInstance inst, svkDevice& device, void* hinstance, void* hwnd);
void					svkWaitQueue					(svkDevice&);
void					svkDestroySurface				(VkInstance, svkDevice&, svkSurface&);
void					svkRefreshSurfaceSize			(svkDevice&, svkSurface&);
svkSwapchain			svkCreateSwapchain				(svkDevice& device, const svkSurface& surface, const svkSwapchain& oldSwapchain, int imageCount = 3, bool deleteOldSwapchainHandle = true);
void					svkDestroySwapchain				(svkDevice& device, svkSwapchain& swapchain, bool deleteSelfHandle = true);
VkCommandPool			svkCreateCommandPool			(svkDevice& device, bool needResetIndividual);
VkCommandBuffer			svkAllocCommandBuffer			(svkDevice& device);
void					svkAllocCommandBuffer			(svkDevice& device, VkCommandPool pool, bool isPrimary, int count, VkCommandBuffer* output);
void					svkFreeCommandBuffer			(svkDevice& device, VkCommandBuffer commandBuffer);
void					svkBeginCommandBuffer			(VkCommandBuffer, bool oneTime = false);
void					svkEndCommandBuffer				(svkDevice&, VkCommandBuffer);
void					svkBeginSecondaryCommandBuffer	(VkCommandBuffer&, const VkRenderPass& renderPass, const VkFramebuffer& framebuffer);

// TODO png支持使用 gfx 代码接入引擎
// TODO 当文件的 pitch 和 vulkan createImage 的 pitch 不一致的时候，需要手动重新拷贝一次，而不能直接使用 map memory 返回的指针来创建。
svkTexture				svkCreateTexture				(svkDevice&, const char* const filename, VkCommandBuffer commandBuffer);
svkTexture				svkCreateTexture				(svkDevice& device, const int width, const int height, VkCommandBuffer outCommandBuffer);
// TODO 没有检查目标 texutrre 的大小和是否可以写入
void					svkCopyTexture					(svkDevice& device, svkTexture& texture, const void* const data, const int sizeofData);
void					svkDestroyTexture				(svkDevice&, svkTexture&);

//shader
svkShaderProgram		svkCreateShaderProgramFromCode	(svkDevice&, const char* const vertCode, const char* const tcsCode, const char* const tesCode, const char* const geoCode, const char* const fragCode, const char* const compCode);
svkShaderProgram		svkCreateShaderProgramFromFile	(svkDevice&, const char* const vertFilename, const char* const tcsFilename, const char* const tesFilename, const char* const geoFilename, const char* const fragFilename, const char* const compFilename);
void					svkDestroyShaderProgram			(svkDevice&, svkShaderProgram&);

// TODO 先创建buffer再拷贝功能没有实现
svkBuffer				svkCreateVertexBuffer			(svkDevice&, const void* data, const int dataSize);
svkBuffer				svkCreateIndexBuffer			(svkDevice&, const void* data, const int dataSize);
svkBuffer				svkCreateUniformBuffer			(svkDevice&, void* data, const int dataSize);
void					svkCopyBuffer					(svkDevice&, svkBuffer&, const void* data, const int dataSize);
void*					svkMapBuffer					(svkDevice&, svkBuffer&);
void					svkUnmapBuffer					(svkDevice&, svkBuffer&);
void					svkDestroyBuffer				(svkDevice&, svkBuffer& buffer);
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
svkPipeline				svkCreatePipeline				(svkDevice& device, VkDescriptorSetLayout pipelineLayout, VkRenderPass renderPass, VkPipelineCache* cache);
svkPipeline				svkCreatePipelineEx				(svkDevice& device, VkDescriptorSetLayout pipelineLayout, VkRenderPass renderPass, VkPrimitiveTopology topology, VkPipelineVertexInputStateCreateInfo& viCreateInfo, svkShaderProgram& shaderProgram, VkPipelineCache* cache);
void					svkDestroyPipeline				(svkDevice&, svkPipeline&, bool deleteCache = true);
void					svkCmdBeginRenderPass			(VkCommandBuffer cb, float clearColorR, float clearColorG, float clearColorB, float clearColorA, float depth, unsigned int stencil, VkRenderPass renderPass, VkFramebuffer framebuffer, uint32_t width, uint32_t height, bool useSecondaryCommandBuffer);
void					svkCmdSetViewPortCubic			(VkCommandBuffer cb, uint32_t width, uint32_t height);
void					svkCmdSetViewPortByGLParams		(VkCommandBuffer cb, const int x, const int y, const int width, int height);
void					svkCmdSetViewPortDirectly		(VkCommandBuffer cb, const int x, const int y, const int width, int height);
void					svkCmdBindVertexBuffers			(VkCommandBuffer cb, int firstBinding, void** vertexBuffers, int attrCount);
void					svkCmdSetScissor				(VkCommandBuffer cb, uint32_t width, uint32_t height);
typedef void			(*presentResultCallback)		(void* userData, VkResult);
int						svkAcquireNextImage				(svkDevice& device, svkSwapchain& swapchain, svkFrame* frames, const int frameIndex, void* userData, presentResultCallback callback);
void					svkQueueSubmit					(svkDevice& device, const VkCommandBuffer* commandBuffers, const int commandBufferCount, svkFrame* frames, const int frame, const int prevFrame);
void					svkQueueSubmitFrame				(svkDevice& device, svkFrame* frames, const int frame, const int prevFrame);
void					svkPresent						(svkDevice& device, svkSwapchain& swapchain, svkFrame* frames, const int frame,  void* userData, presentResultCallback callback);
//void					svkSubmitAndPresent				(svkDevice& device, svkSwapchain& swapchain, const int frameIndex, void* userData, presentResultCallback callback);
VkRenderPass			svkCreateRenderPass				(svkDevice& device, VkFormat format, VkFormat depthFormat, VkImageLayout colorAttachmentFinalLayout);
VkFramebuffer			svkCreateFrameBuffer			(svkDevice& device, VkRenderPass renderPass, VkImageView* attachments, const int attachmentCount, const uint32_t width, const uint32_t height);
svkImage				svkCreateAttachmentDepthImage	(svkDevice& device, VkFormat format, const int width, const int height);
svkImage				svkCreateAttachmentColorImage	(svkDevice& device, VkFormat format, const int width, const int height);
void					svkDestroyImage					(svkDevice& device, svkImage& image);
int						svkCreateFrames					(svkDevice& device, svkSwapchain& swapchain, VkImageView depthImageView, VkRenderPass renderPass, const int width, const int height, svkFrame* outputFrames, const int outputFrameCapacity);
void					svkDestroyFrames				(svkDevice& device, svkFrame* frames, const int frameCount);


