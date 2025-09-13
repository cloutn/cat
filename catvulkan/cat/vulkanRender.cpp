#include "cat/vulkanRender.h"

#include "cat/descriptorAllocator.h"
#include "cat/commandAllocator.h"
#include "cat/xxhash.h"

#include "libimg/image.h"

#include "scl/stringdef.h"
#include "scl/log.h"
#include "scl/math.h"


#include "imgui/imgui.h"
#include "imgui_impl_vulkan.h"

#include <assert.h>

#define memclr(s) memset(&s, 0, sizeof(s))

#ifdef DEVICE_TYPE
    #undef DEVICE_TYPE
#endif

#ifdef _DEBUG
static const bool ENABLE_VALIDATION_LAYER = true;
#else
static const bool ENABLE_VALIDATION_LAYER = false;
#endif

#ifndef countof
#define countof(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

namespace cat {

VkPipelineVertexInputStateCreateInfo	_buildVulkanVertexInput		(const VertexAttr* attrs, const int attrCount, void** vertexBuffers, VkVertexInputBindingDescription* viBinds, VkVertexInputAttributeDescription* viAttrs);
VkPrimitiveTopology						_toVulkanPrimitiveTopology	(PRIMITIVE_TYPE type);

VulkanRender::VulkanRender()  : 
	m_inst						(NULL), 
	m_mainRenderPass			(NULL),
	m_frameCount				(0),
	m_colorFormat				(VK_FORMAT_UNDEFINED),
	m_depthFormat				(VK_FORMAT_UNDEFINED),
	m_pickRenderPass			(NULL),
	m_pickCommandBuffer			(NULL),
	m_pickCommandAllocator		(NULL),
	m_pickFence					(NULL),
	m_pickSemaphore				(NULL),
	m_pickCopyCommandBuffer		(NULL),
	m_isInit					(false),
	m_minimized					(false),
	m_frameIndex				(-1),
	m_prevFrameIndex			(-1),
	m_matrixChanged				(false),
	m_scale						(1.0f),
	m_frameUniformBufferOffset	(0),
	m_reverseZ					(false),
	m_windowInstance			(NULL),
	m_windowHandle				(NULL),
	m_IMGUIDescriptorPool		(NULL),
	m_bindCommandBuffer			(NULL)
	
{
	memclr(m_device);
	memclr(m_surface);
	memclr(m_swapchain);
	memclr(m_device);
	memclr(m_surface);
	memclr(m_frameUniforms);
	memclr(m_frameUniformBuffersMapped);
	//memclr(m_clearColor);
	memclr(m_frames);
	memclr(m_drawContext);
	memclr(m_pickRenderTarget);
	memclr(m_deviceInfo);
	memclr(m_commandAllocator);
	memclr(m_mainDepthImage);
	memclr(m_pickPassImageCPUBuffer);
	//m_pickImageSize.clear();
	//m_pickImageOffset.clear();
}


bool VulkanRender::init(void* hInstance, void* hwnd)
{
	if (is_init())
		return false;

	// init hash tables
	m_pipelines.init			(MAX_PIPELINE_COUNT * MAX_CONFLICT);
	m_descriptorAllocators.init	(MAX_DESCRITOR_ALLOCATOR_COUNT * MAX_CONFLICT);
	m_descriptorSetCache.init	(MAX_DESCRIPTOR_SET_CACHE_SIZE * MAX_CONFLICT);

	m_windowInstance			= hInstance;
	m_windowHandle				= hwnd;

	m_inst						= svkCreateInstance	(ENABLE_VALIDATION_LAYER);
	m_device					= svkCreateDevice	(m_inst);
	m_surface					= svkCreateSurface	(m_inst, m_device, hInstance, hwnd);

	VkFormat colorFormats[]		= { VK_FORMAT_R8G8B8A8_UNORM };
	m_colorFormat				= svkChooseColorFormat(m_device, colorFormats, countof(colorFormats));
	VkFormat depthFormats[]		= { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM }; 
	m_depthFormat				= svkChooseDepthFormat(m_device, depthFormats, countof(depthFormats));

	_createMainRenderTarget();

	// 3D picking
	m_pickRenderPass			= svkCreateRenderPass(m_device, m_colorFormat, m_depthFormat, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	m_pickRenderTarget			= _createRenderTarget(m_device, m_colorFormat, m_depthFormat, m_pickRenderPass, m_surface.width, m_surface.height);
	m_pickCommandBuffer			= svkAllocCommandBuffer(m_device);
	m_pickCopyCommandBuffer		= svkAllocCommandBuffer(m_device);
	m_pickCommandAllocator		= new CommandAllocator();
	m_pickCommandAllocator->init(m_device);
	m_pickFence					= svkCreateFence(m_device, true);
	m_pickSemaphore				= svkCreateSemaphore(m_device);
	//m_pickImageSize.set(10, 10);
	//m_pickImageSize.set(m_surface.width, m_surface.height);
	//m_pickImageOffset.set(m_surface.width / 2 - m_pickImageSize.x / 2, m_surface.height / 2 - m_pickImageSize.y / 2);
	m_pickPassImageCPUBuffer	= svkCreateBuffer(m_device, VK_BUFFER_USAGE_TRANSFER_DST_BIT, m_surface.width * m_surface.height * 4);

	scl::matrix mvp = scl::matrix::identity();

	for (int i = 0; i < static_cast<int>(m_swapchain.imageCount); ++i)
	{
		if (NULL != m_frameUniforms[i].buffer)
		{
			svkUnmapBuffer(m_device, m_frameUniforms[i]);
			svkDestroyBuffer	(m_device, m_frameUniforms[i]);
		}

		int minUniformBufferOffset		= static_cast<int>(m_device.gpuProperties.limits.minUniformBufferOffsetAlignment);
		int maxBytesPerFrame			= minUniformBufferOffset * MAX_OBJECT_PER_FRAME * MAX_MATRIX_PER_FRAME;
		m_frameUniforms[i]				= svkCreateUniformBuffer(m_device, NULL, maxBytesPerFrame);
		m_frameUniformBuffersMapped[i]	= svkMapBuffer			(m_device, m_frameUniforms[i]);
	}

	for (int i = 0; i < static_cast<int>(m_swapchain.imageCount); ++i)
	{
		m_commandAllocator[i] = new CommandAllocator();
		m_commandAllocator[i]->init(m_device);
	}

	_fillDeviceInfo(m_device, m_deviceInfo);
	m_isInit = true;

	return true;
}

//static int ImGui_CreateVkSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
//{
//    //ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
//    //ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
//    //IM_UNUSED(bd);
//    //IM_ASSERT(bd->ClientApi == GlfwClientApi_Vulkan);
//
//	ImGuiIO& io = ImGui::GetIO();
//	VulkanRender* render = reinterpret_cast<VulkanRender*>(io.BackendRendererUserData);
//
//
//#if defined(VK_USE_PLATFORM_WIN32_KHR)
//	HINSTANCE hInstance		= ::GetModuleHandle(0);
//
//	VkWin32SurfaceCreateInfoKHR createInfo;
//	memclr(createInfo);
//	createInfo.sType		= VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
//	createInfo.pNext		= NULL;
//	createInfo.flags		= 0;
//	createInfo.hinstance	= ::GetModuleHandle(0);
//	createInfo.hwnd			= (HWND)viewport->PlatformHandleRaw;
//
//	VkSurfaceKHR deviceSurfaceHandle;
//	VkResult err = vkCreateWin32SurfaceKHR((VkInstance)vk_instance, &createInfo, NULL, &deviceSurfaceHandle);
//	assert(err = VK_SUCCESS);
//    return (int)err;
//#endif
//}

void VulkanRender::initIMGUI()
{
	int countPerType[VK_DESCRIPTOR_TYPE_RANGE_SIZE] = { 0 };
	for (int i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i)
		countPerType[i]			= 1000;
	m_IMGUIDescriptorPool		= svkCreateDescriptorPoolEx2(m_device, 1000, countPerType);

	// Setup Dear ImGui context
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance			= m_inst;
	init_info.PhysicalDevice	= m_device.gpu;
	init_info.Device			= m_device.device;
	init_info.QueueFamily		= m_device.queueFamilyIndex;
	init_info.Queue				= m_device.queue;
	init_info.PipelineCache		= NULL;
	init_info.DescriptorPool	= m_IMGUIDescriptorPool;
	init_info.Subpass			= 0;
	init_info.MinImageCount		= m_swapchain.imageCount;
	init_info.ImageCount		= m_swapchain.imageCount;
	init_info.MSAASamples		= VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator			= NULL;
	init_info.CheckVkResultFn	= NULL;

	//ImGuiIO&			io				= ImGui::GetIO();
	//io.BackendRendererUserData			= this;
	//ImGuiPlatformIO&	platformIO		= ImGui::GetPlatformIO();
	//platformIO.Platform_CreateVkSurface	= ImGui_CreateVkSurface;

	ImGui_ImplVulkan_Init		(&init_info, m_mainRenderPass);

	// Upload Fonts
	VkCommandBuffer textureCommandBuffer		= svkAllocCommandBuffer(m_device);
	svkBeginCommandBuffer						(textureCommandBuffer);;
	ImGui_ImplVulkan_CreateFontsTexture			(textureCommandBuffer);
	svkEndCommandBufferAndSubmit				(m_device, textureCommandBuffer);
	ImGui_ImplVulkan_DestroyFontUploadObjects	();
}


VulkanRender::~VulkanRender()
{
	scl::varray<svkPipeline*> pipelines;
	pipelines.reserve(64);
	m_pipelines.get_values(pipelines);
	for (int i = 0; i < pipelines.size(); ++i)
	{
		if (NULL == pipelines[i])
			continue;
		svkDestroyPipeline(m_device, *pipelines[i], true);
		delete pipelines[i];
	}
	for (int i = 0; i < static_cast<int>(m_swapchain.imageCount); ++i)
	{
		svkUnmapBuffer	(m_device, m_frameUniforms[i]);
		svkDestroyBuffer(m_device, m_frameUniforms[i]);
	}
	for (scl::hash_table<int, DescriptorAllocator*>::iterator it = m_descriptorAllocators.begin(); 
		it != m_descriptorAllocators.end(); 
		++it)
	{
		DescriptorAllocator* allocator = it->value;
		allocator->release();
		delete allocator;
	}

	for (uint i = 0; i < m_swapchain.imageCount; ++i)
	{
		m_commandAllocator[i]->release(m_device);
		delete m_commandAllocator[i];
	}

	m_pickCommandAllocator->release(m_device);
	delete m_pickCommandAllocator;

	svkFreeCommandBuffer(m_device, m_pickCommandBuffer);	
	svkFreeCommandBuffer(m_device, m_pickCopyCommandBuffer);	
	svkDestroyFence		(m_device, m_pickFence);
	svkDestroySemaphore	(m_device, m_pickSemaphore);
	svkDestroyBuffer	(m_device, m_pickPassImageCPUBuffer);

	//_destroyPickRenderTarget();
	_destroyRenderTarget(m_device, m_pickRenderTarget);
	svkDestroyRenderPass(m_device, m_pickRenderPass);

	svkDestroyRenderPass(m_device, m_mainRenderPass);
	svkDestroyFrames	(m_device, m_frames, m_frameCount);
	svkDestroyImage		(m_device, m_mainDepthImage);
	svkDestroySwapchain	(m_device, m_swapchain, true);
	svkDestroySurface	(m_inst, m_device, m_surface);
	svkDestroyDevice	(m_device);
	svkDestroyInstance	(m_inst);
}

void* VulkanRender::createVertexBuffer(const int bufferSize)
{
	svkBuffer* buf = new svkBuffer { NULL, NULL };
	return buf;
}

void VulkanRender::releaseVertexBuffer(void* vertexBuffer)
{
	if (NULL == vertexBuffer)
		return;

	//log_debug_unsafe("release buffer = %x", vertexBuffer);
	svkDestroyBuffer(m_device, *static_cast<svkBuffer*>(vertexBuffer));
	delete vertexBuffer;
}

void VulkanRender::writeVertexBuffer(const void* src, void* dstVertexBuffer, const int sizeInByte)
{
	if (NULL == dstVertexBuffer || NULL == src || sizeInByte <= 0)
		return;

	svkBuffer* dstBuffer = static_cast<svkBuffer*>(dstVertexBuffer);
	if (NULL != dstBuffer->buffer)
	{
		svkWriteBuffer(m_device, *dstBuffer, src, sizeInByte);
	}
	else
	{
		*dstBuffer = svkCreateVertexBuffer(m_device, src, sizeInByte);
	}
}

void VulkanRender::readVertexBuffer(void* dst, void* srcVertexBuffer, const int sizeInByte, const int offset)
{
	if (NULL == srcVertexBuffer || NULL == dst || sizeInByte <= 0)
		return;

	svkBuffer* srcBuffer = static_cast<svkBuffer*>(srcVertexBuffer);
	
	// 使用带 offset 的 svkMapMemory 版本
	void* src = svkMapMemory(m_device, srcBuffer->memory, static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(sizeInByte));
	memcpy(dst, src, sizeInByte);
	svkUnmapMemory(m_device, srcBuffer->memory);
}

//void* VulkanRender::mapVertexBuffer(void* vertexBuffer)
//{
//	if (NULL == vertexBuffer)
//		return NULL;
//
//	svkBuffer* buf = static_cast<svkBuffer*>(vertexBuffer);
//	void* mapmm = svkMapBuffer(m_device, *buf);
//	return mapmm;
//}
//
//void VulkanRender::unmapVertexBuffer(void* vertexBuffer)
//{
//	if (NULL == vertexBuffer)
//		return;
//
//	svkBuffer* buf = static_cast<svkBuffer*>(vertexBuffer);
//	svkUnmapBuffer(m_device, *buf);
//}

void* VulkanRender::createIndexBuffer(const int bufferSize)
{
  svkBuffer* buf = new svkBuffer { NULL, NULL };
  return buf;
}

//如果发生一个 buffer 被两次释放的问题，原因是多个 primitive 共享了一个indexbuffer
void VulkanRender::releaseIndexBuffer(void* indexBuffer)
{
	if (NULL == indexBuffer)
		return;

	svkDestroyBuffer(m_device, *static_cast<svkBuffer*>(indexBuffer));
	delete indexBuffer;
}


void VulkanRender::writeIndexBuffer(const void* src, void* dstIndexBuffer, const int sizeInByte)
{
	if (NULL == dstIndexBuffer)
		return;
	svkBuffer* dstBuffer = static_cast<svkBuffer*>(dstIndexBuffer);
	if (NULL != dstBuffer->buffer)
	{
		svkWriteBuffer(m_device, *dstBuffer, src, sizeInByte);
	}
	else
	{
		*dstBuffer = svkCreateIndexBuffer(m_device, src, sizeInByte);
	}
}

void VulkanRender::readIndexBuffer(void* dst, void* srcIndexBuffer, const int sizeInByte, const int offset)
{
	if (NULL == srcIndexBuffer || NULL == dst || sizeInByte <= 0)
		return;

	svkBuffer* srcBuffer = static_cast<svkBuffer*>(srcIndexBuffer);
	
	// 使用带 offset 的 svkMapMemory 版本
	void* src = svkMapMemory(m_device, srcBuffer->memory, static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(sizeInByte));
	memcpy(dst, src, sizeInByte);
	svkUnmapMemory(m_device, srcBuffer->memory);
}

//void* VulkanRender::mapIndexBuffer(void* indexBuffer)
//{
//	if (NULL == indexBuffer)
//		return NULL;
//
//	svkBuffer* buf = static_cast<svkBuffer*>(indexBuffer);
//	void* mapmm = svkMapBuffer(m_device, *buf);
//	return mapmm;
//}
//
//void VulkanRender::unmapIndexBuffer(void* indexBuffer)
//{
//	if (NULL == indexBuffer)
//		return;
//
//	svkBuffer* buf = static_cast<svkBuffer*>(indexBuffer);
//	svkUnmapBuffer(m_device, *buf);
//}

void* VulkanRender::createTexture(const char* const filename, int* width, int* height, int* pitch, PIXEL* pixel)
{
	//char s[256] = { 0 };
	//scl::wchar_to_ansi(s, 256, filename, static_cast<int>(wcslen(filename)), scl::Encoding_UTF8);

	//创建纹理   
	svkTexture* tex = new svkTexture; 
	*tex = svkCreateTexture(m_device, filename, NULL, img::load_img_to_buffer_or_get_size); 

	return reinterpret_cast<void*>(tex);
}

unsigned char* VulkanRender::loadImage(const char* const filename, int* width, int* height, int* pitch, PIXEL* pixel)
{
	assert(false && "not implemented!");
	return NULL;
	//int _pix = -1;
	//unsigned char* data = gfx::load_png_data(filename, width, height, pitch, &_pix);
	//if (_pix == 4)
	//	*pixel = gfx::PIXEL_RGBA;
	//else if (_pix == 3)
	//	*pixel = gfx::PIXEL_RGB;

	//return data;
}


void* VulkanRender::createTexture(const int width, const int height, const PIXEL pixel)
{
	assert(false); //the text must be clear with (0, 0, 0)!!!

	svkTexture* tex = new svkTexture;
	*tex = svkCreateTexture(m_device, width, height, NULL);
	return reinterpret_cast<void*>(tex);
}

void VulkanRender::copyTexture(void* texture, const int offset_x, const int offset_y, const int width, const int height, const void* data, const PIXEL pixel, const int alignment)
{
	assert(false && "not implemented!");

	//svkTexture* tex = static_cast<svkTexture*>(texture);

	//svkCopyTexture(m_device, *tex, 
}

void VulkanRender::releaseTexture(void* texture)
{
	if (NULL == texture)
		return;
	svkTexture* tex = static_cast<svkTexture*>(texture);
	svkDestroyTexture(m_device, *tex);
	delete tex;
}

int VulkanRender::getDeviceWidth() const
{
	return m_surface.width;
}

int VulkanRender::getDeviceHeight() const
{
	return m_surface.height;
}

void VulkanRender::saveTexture(void* texture, const char* const filename)
{
}

//void VulkanRender::prepare()
//{
//
//}


//void VulkanRender::unprepare()
//{
//	//svkDestroyPipeline(m_device, m_pipeline, true);
//	//svkDestroyDescriptorCreator(m_device, m_descriptorCreator);
//	svkDestroySwapchain(m_device, m_swapchain, true);
//}

void presentCallback(void* userdata, VkResult err)
{
	VulkanRender* render = static_cast<VulkanRender*>(userdata);

	if (err == VK_ERROR_OUT_OF_DATE_KHR) 
	{
		render->waitIdle();
		render->recreateSwapchain();
	}
	else if (err == VK_ERROR_SURFACE_LOST_KHR) 
	{
		render->waitIdle();
		render->recreateSurface();
		render->recreateSwapchain();
	}
	else if (err == VK_SUBOPTIMAL_KHR) 
	{
		/// demo->swapchain is not as optimal as it could be, but the platform's
		/// presentation engine will still present the image correctly.
	}
	else
		assert(!err);
}

void VulkanRender::swap()
{
	if (_minimized())
	{
		// 这里的逻辑是为了处理以下情况：
		//		最小化之后，整个render会不再执行。
		//		这时需要调用 recreateSwapchain 不断刷新屏幕的大小检测是否恢复了正常窗口。
		//		当然在 windows下通过 WM_SIZE 消息来通知也可以。
		recreateSwapchain();
		return;
	}

	//for (int i = 0; i < m_currentFrameCommandBuffers.size(); ++i)
	//{
	//	svkQueueSubmit(
	//		m_device, 
	//		&m_currentFrameCommandBuffers[i],
	//		1,
	//		//m_currentFrameCommandBuffers.c_array(), 
	//		//m_currentFrameCommandBuffers.size(), 
	//		xxxx m_frames[m_prevFrameIndex].imageAcquireSemaphore,  
	//		xxxx m_frames[m_frameIndex].drawCompleteSemaphore, 
	//		//m_frames[m_frameIndex].fence);
	//		m_currentFrameFences[i]);
	//}

	svkPresent(m_device, m_swapchain, m_frames, m_frameIndex, this, presentCallback);
}

void VulkanRender::clear()
{
	// in vulkan, clear is finished in render pass.
}

void* VulkanRender::createShader(int shaderType)
{
	// shader 
	uint shader = 0;
	svkPipeline* pipeline = new svkPipeline;
	switch (shaderType)
	{
	case 0: 
		{
			assert(false);
		}
		break;
	default : assert(false); break;
	}
	return reinterpret_cast<void*>(pipeline);
}


void* VulkanRender::createShader(const char* const vs_code, const char* const ps_code)
{
	//assert(false);
	svkShaderProgram* shaderProgram = new svkShaderProgram;
	*shaderProgram = svkCreateShaderProgramFromCode(m_device, vs_code, NULL, NULL, NULL, ps_code, NULL);
	return shaderProgram;
}

void VulkanRender::releaseShader(void* _shader)
{
	svkShaderProgram* shaderProgram = static_cast<svkShaderProgram*>(_shader);
	svkDestroyShaderProgram(m_device, *shaderProgram);
	delete shaderProgram;
}

void VulkanRender::release()
{

}

void VulkanRender::beginDraw()
{
	if (_minimized())
		return;

	m_prevFrameIndex					= m_frameIndex;
	m_frameIndex						= svkAcquireNextImage(m_device, m_swapchain, m_frames, m_frameIndex, this, presentCallback);


	m_frameUniformBufferOffset			= 0;
}

void VulkanRender::endDraw()
{
	if (_minimized())
		return;
}

void VulkanRender::beginPickPass(scl::vector4& clearColorRGBA)
{
	if (!svkIsFenceSignaled(m_device, m_pickFence))
		return;

	m_drawContext.renderPass			= m_pickRenderPass;
	m_drawContext.framebuffer			= m_pickRenderTarget.framebuffer;
	m_drawContext.width					= m_surface.width;
	m_drawContext.height				= m_surface.height;
	m_drawContext.commandAllocator		= m_pickCommandAllocator;
	m_drawContext.uniform				= m_frameUniforms[m_frameIndex];
	m_drawContext.uniformBufferMapped	= m_frameUniformBuffersMapped[m_frameIndex];
	m_drawContext.clearColorRGBA		= clearColorRGBA;

	m_drawContext.commandAllocator->reset(m_device);
}

scl::vector4 VulkanRender::endPickPass(int x, int y)
{
	if (NULL == m_drawContext.renderPass)
		return scl::vector4{0};

	VkResult err;

	// scene command buffer
	VkCommandBuffer& primaryCb = m_pickCommandBuffer;
	svkResetCommandBuffer(primaryCb);
	svkBeginCommandBuffer(primaryCb);
	scl::vector4& clearColor = m_drawContext.clearColorRGBA;
	svkCmdBeginRenderPass(primaryCb, clearColor.r, clearColor.g, clearColor.b, clearColor.a, m_reverseZ ? 0 : 1.0f, 0, m_drawContext.renderPass, m_drawContext.framebuffer, m_drawContext.width, m_drawContext.height, true);
	
	CommandAllocator* commandAllocator = m_drawContext.commandAllocator;
	if (commandAllocator->getAllocCount() > 0)
		vkCmdExecuteCommands(primaryCb, commandAllocator->getAllocCount(), commandAllocator->getAllocArray());

	vkCmdEndRenderPass(primaryCb);
	err = svkEndCommandBuffer(primaryCb);
	assert(!err);

	svkQueueSubmit(
		m_device,
		&m_pickCommandBuffer,
		1,
		NULL,
		&m_pickSemaphore,
		NULL);

	const scl::vector2i pickSize { 1, 1 };
	const scl::vector2i pickOffset { x - pickSize.x / 2, y - pickSize.y /  2 };

	// copy image to buffer
	VkCommandBuffer tmpCB = m_pickCopyCommandBuffer;
	svkResetCommandBuffer(tmpCB);
	svkBeginCommandBuffer(tmpCB, true);

	VkBufferImageCopy region;
	memclr(region);
	region.bufferOffset			= 0;
	region.bufferRowLength		= pickSize.x;
	region.bufferImageHeight	= 0;
	region.imageSubresource		= { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.imageOffset			= { pickOffset.x, pickOffset.y, 0 };
	region.imageExtent			= { (unsigned int)pickSize.x, (unsigned int)pickSize.y, 1 };
	vkCmdCopyImageToBuffer(tmpCB, m_pickRenderTarget.colorImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_pickPassImageCPUBuffer.buffer, 1, &region);

	svkEndCommandBuffer(tmpCB);

	svkQueueSubmit(
		m_device,
		&tmpCB,
		1,
		&m_pickSemaphore,
		NULL,
		m_pickFence);

	memclr(m_drawContext);

	return savePickPass(pickSize.x, pickSize.y);
}

scl::vector4 VulkanRender::savePickPass(int width, int height)
{
	svkWaitFence(m_device, &m_pickFence, 1);

	int			copied	= 0;
	//const int	BYTES	= m_surface.width * m_surface.height * 4;
	uint8*		data	= (uint8*)svkMapBuffer(m_device, m_pickPassImageCPUBuffer, -1);

	int			x		= width / 2;
	int			y		= height / 2;
	uint8*		point = &data[(y * width + x) * 4];
	scl::vector4 r{};
	r.r = point[0] / 255.0;
	r.g = point[1] / 255.0;
	r.b = point[2] / 255.0;
	r.a = point[3] / 255.0;

	//static int i = 0;
	//string32 fname;
	//fname.format("d:/testCat_%d.bmp", i++);
	//FILE* f = fopen(fname.c_str(), "wb");
	//img::save_bmp(f, m_pickImageSize.x, m_pickImageSize.y, 0, data);
	//img::save_bmp(f, width, height, 0, data);

	svkUnmapBuffer(m_device, m_pickPassImageCPUBuffer);

	//fclose(f);
	return r;
}

void VulkanRender::beginScenePass(scl::vector4& clearColorRGBA)
{
	if (_minimized())
		return;

	svkWaitFence(m_device, &m_frames[m_frameIndex].fence, 1);

	// draw context
	m_drawContext.renderPass			= m_mainRenderPass;
	m_drawContext.framebuffer			= m_frames[m_frameIndex].framebuffer;
	m_drawContext.width					= m_surface.width;
	m_drawContext.height				= m_surface.height;
	m_drawContext.uniform				= m_frameUniforms[m_frameIndex];
	m_drawContext.uniformBufferMapped	= m_frameUniformBuffersMapped[m_frameIndex];
	m_drawContext.commandAllocator		= m_commandAllocator[m_frameIndex];
	m_drawContext.clearColorRGBA		= clearColorRGBA;

	m_drawContext.commandAllocator->reset(m_device);
}

void VulkanRender::endScenePass()
{
	if (_minimized())
		return;

	VkResult err;

	VkCommandBuffer& primaryCb = m_frames[m_frameIndex].commandBuffer;
	svkBeginCommandBuffer(primaryCb);
	scl::vector4& clearColor = m_drawContext.clearColorRGBA;
	svkCmdBeginRenderPass(primaryCb, clearColor.r, clearColor.g, clearColor.b, clearColor.a, m_reverseZ ? 0 : 1.0f, 0, m_drawContext.renderPass, m_drawContext.framebuffer, m_drawContext.width, m_drawContext.height, true);
	
	CommandAllocator* commandAllocator = m_commandAllocator[m_frameIndex];
	if (commandAllocator->getAllocCount() > 0)
		vkCmdExecuteCommands(primaryCb, commandAllocator->getAllocCount(), commandAllocator->getAllocArray());

	vkCmdEndRenderPass(primaryCb);
	err = svkEndCommandBuffer(primaryCb);
	assert(!err);

	svkQueueSubmit(
		m_device,
		&primaryCb,
		1,
		&m_frames[m_prevFrameIndex].imageAcquireSemaphore,
		&m_frames[m_frameIndex].drawCompleteSemaphore,
		m_frames[m_frameIndex].fence);

	memclr(m_drawContext);
}

VkIndexType _toVkIndexType(int indexComponentType)
{
	switch (indexComponentType)
	{
	case ELEM_TYPE_UINT16: return VK_INDEX_TYPE_UINT16;
	case ELEM_TYPE_UINT32: return VK_INDEX_TYPE_UINT32;
	default: assert(false); break;
	}
	return VK_INDEX_TYPE_UINT16;
}

int VulkanRender::_alignUniformBufferOffset(int size)
{
	int minOffset = static_cast<int>(m_device.gpuProperties.limits.minUniformBufferOffsetAlignment);
	//int minOffset = 256; // debug test
	int div = size / minOffset;
	int mod = size % minOffset;
	if (mod > 0)
		++div;
	return div * minOffset;
}

int VulkanRender::_fillUniformData(
	const scl::matrix&	mvp,
	void*				texture,
	const int			jointMatrixCount,
	svkDescriptorData*	uniformDatas,
	const int			uniformDataCapacity)
{
	assert(uniformDataCapacity >= 3);

	//这里是具体绑定的 mvp 矩阵数据和纹理数据。

	memset(uniformDatas, 0, sizeof(svkDescriptorData) * uniformDataCapacity);
	const int MAX_INFO_COUNT = countof(uniformDatas[1].data);

	// [0]
	int idx = 0;
	uniformDatas[idx].data[0].buffer.buffer		= m_drawContext.uniform.buffer;
	//uniformDatas[idx].data[0].buffer.bufferSize	= sizeof(mvp);
	//uniformDatas[idx].data[0].buffer.bufferSize = m_device.gpuProperties.limits.maxUniformBufferRange - 1 - dynamicOffsets[0];
	// !!! Set buffer size to max limits, so all the descriptor has the same size, we can use less descriptor.
	uniformDatas[idx].data[0].buffer.bufferSize = m_device.gpuProperties.limits.maxUniformBufferRange - 1;
	uniformDatas[idx].dataCount = 1;
	uniformDatas[idx].binding = 0;
	++idx;

	// [1]
	if (jointMatrixCount > 0)
	{
		//uniformDatas[idx].dataCount					= min(jointMatrixCount, MAX_INFO_COUNT);	
		//for (int i = 0; i < uniformDatas[idx].dataCount; ++i)
		//{
		//	uniformDatas[idx].data[i].buffer.buffer	= m_drawContext.uniform.buffer;
		//	uniformDatas[idx].data[i].buffer.bufferSize = sizeof(scl::matrix) * jointMatrixCount;
		//}s
		uniformDatas[idx].data[0].buffer.buffer		= m_drawContext.uniform.buffer;
		//uniformDatas[idx].data[0].buffer.bufferSize = sizeof(scl::matrix) * jointMatrixCount;
		//uniformDatas[idx].data[0].buffer.bufferSize = m_device.gpuProperties.limits.maxUniformBufferRange - 1 - dynamicOffsets[1];
		uniformDatas[idx].data[0].buffer.bufferSize = m_device.gpuProperties.limits.maxUniformBufferRange - 1;
		uniformDatas[idx].dataCount					= 1;
		uniformDatas[idx].binding					= 2;
		++idx;
	}

	// [2]
	if (NULL != texture)
	{
		const svkTexture* _svkTexture				= static_cast<const svkTexture*>(texture);
		uniformDatas[idx].data[0].texture.sampler	= _svkTexture->sampler;
		uniformDatas[idx].data[0].texture.imageView	= _svkTexture->view;
		uniformDatas[idx].dataCount					= 1;
		uniformDatas[idx].binding					= 1;
		++idx;
	}
	return idx;
}

DescriptorSet VulkanRender::_prepareDescriptorSet(
	void*					shader, 
	svkDescriptorData*		descriptorDatas,
	const int				descriptorDataCount,
	VkDescriptorSetLayout&	descriptorSetLayout,
	uint32_t*				dynamicOffsets,
	uint32					dynamicOffsetCount)
{
	svkShaderProgram* shaderProgram = static_cast<svkShaderProgram*>(shader);
	DescriptorAllocator*	descriptorAllocator = NULL;
	DescriptorSet			descriptorSet = { 0 };

	VkDescriptorSetLayoutBinding*	layoutBinds		= shaderProgram->descriptorSetLayoutBinds;
	int								layoutBindCount	= shaderProgram->descriptorSetLayoutBindCount;
	if (NULL == shaderProgram->descriptorSetLayoutBinds || shaderProgram->descriptorSetLayoutBindCount <= 0)
		return descriptorSet;

	int			descriptorSetLayoutHash = XXH32(layoutBinds, sizeof(VkDescriptorSetLayoutBinding) * layoutBindCount, 0);	
	const int	findIndex				= m_descriptorAllocators.find_index(descriptorSetLayoutHash);
	if (findIndex != -1)
	{
		descriptorAllocator	= m_descriptorAllocators.get_value(findIndex);
	}
	else
	{
		descriptorAllocator = new DescriptorAllocator();
		descriptorAllocator->init(m_device, layoutBinds, layoutBindCount);
		m_descriptorAllocators.add(descriptorSetLayoutHash, descriptorAllocator);
	}

	DescriptorDataKey dataKey;
	dataKey.init(layoutBinds, layoutBindCount, descriptorDatas, m_drawContext.uniform, dynamicOffsets, dynamicOffsetCount);
	int cacheIndex = m_descriptorSetCache.find_index(dataKey);
	if (cacheIndex != -1)
	{
		descriptorSet = m_descriptorSetCache.get_value(cacheIndex);
	}
	else
	{
		descriptorSet = descriptorAllocator->alloc();
		svkUpdateDescriptorSet(m_device, descriptorSet.set, layoutBinds, layoutBindCount, descriptorDatas, descriptorDataCount);
		m_descriptorSetCache.add(dataKey, descriptorSet);
	}
	if (NULL != descriptorAllocator)
		descriptorSetLayout = descriptorAllocator->getLayout();

	return descriptorSet;
}


void VulkanRender::_preparePipeline(
	const int				primitiveType,
	const VertexAttr*		attrs,
	const int				attrCount,
	void**					vertexBuffers,
	VkDescriptorSetLayout	descriptorSetLayout,
	void*					shader,
	VkRenderPass			renderPass,
	svkPipeline*&			pipeline)
{
	VkPrimitiveTopology	vkTopology  = _toVulkanPrimitiveTopology(static_cast<PRIMITIVE_TYPE>(primitiveType));
	PipelineKey			pipelineKey(attrs, shader, vkTopology, reinterpret_cast<void*>(renderPass));
	const int			findIndex	= m_pipelines.find_index(pipelineKey);
	if (findIndex != -1)
	{
		pipeline = m_pipelines.get_value(findIndex);
	}
	else
	{
		//vertex input
		const int MAX_ATTR_COUNT = 256;
		VkVertexInputBindingDescription		viBinds	[MAX_ATTR_COUNT];
		VkVertexInputAttributeDescription	viAttrs	[MAX_ATTR_COUNT];
		memset(viBinds,	0, sizeof(viBinds));
		memset(viAttrs, 0, sizeof(viAttrs));
		VkPipelineVertexInputStateCreateInfo	viCreateInfo	= _buildVulkanVertexInput(attrs, attrCount, vertexBuffers, viBinds, viAttrs);
		svkShaderProgram*						shaderProgram	= static_cast<svkShaderProgram*>(shader);

		pipeline	= new svkPipeline;
		*pipeline	= svkCreatePipelineEx(m_device, descriptorSetLayout, m_drawContext.renderPass, vkTopology, viCreateInfo, *shaderProgram, NULL, m_reverseZ ? VK_COMPARE_OP_GREATER: VK_COMPARE_OP_LESS);
		m_pipelines.add(pipelineKey, pipeline);
	}
}

uint32_t VulkanRender::_fillDynamicOffsets(
	uint32_t*			dynamicOffsets, 
	const int			dynamicOffsetCapacity, 
	const scl::matrix&	mvp,
	const scl::matrix*	jointMatrices,
	const int			jointMatrixCount)
{
	if (dynamicOffsetCapacity < 2)
	{
		assert(false);
		return 0;
	}
	uint32_t dynamicOffsetCount = 0;

	memcpy((byte*)(m_drawContext.uniformBufferMapped) + m_frameUniformBufferOffset, &mvp, sizeof(mvp));
	dynamicOffsets[0]			= m_frameUniformBufferOffset;
	m_frameUniformBufferOffset	+= _alignUniformBufferOffset(sizeof(mvp));
	++dynamicOffsetCount;

	if (jointMatrixCount > 0 && NULL != jointMatrices)
	{
		memcpy((byte*)(m_drawContext.uniformBufferMapped) + m_frameUniformBufferOffset, jointMatrices, sizeof(scl::matrix) * jointMatrixCount);
		dynamicOffsets[1]			= m_frameUniformBufferOffset;
		m_frameUniformBufferOffset	+= _alignUniformBufferOffset(sizeof(scl::matrix) * jointMatrixCount);
		++dynamicOffsetCount;
	}
	return dynamicOffsetCount;
}


void VulkanRender::_prepareDescriptorSetAndFillData(
	void*					shader, 
	const scl::matrix&		mvp,
	void*					texture,
	const scl::matrix*		jointMatrices,
	const int				jointMatrixCount,
	DescriptorSet&			outputDescriptorSet,
	VkDescriptorSetLayout&	outputDescriptorSetLayout,
	uint32_t*				outputDynamicOffsets,
	uint32_t				outputDynamicOffsetCapacity,
	uint32_t&				outputDynamicOffsetCount)
{
	// uniform 数据分为两个部分，
	//		一个是声明信息，就是uniform bind
	//		一个是具体数据信息，就是 uniform data
	//		具体数据要和前面声明的类型对应上。
	svkDescriptorData uniformDatas[3];
	int uniformDataCount		= _fillUniformData		(mvp, texture, jointMatrixCount, uniformDatas, countof(uniformDatas));

	// 这里使用 dynamic offset 进行优化
	// 具体优化思路可以参考 arm 的文章 ：https://community.arm.com/developer/tools-software/graphics/b/blog/posts/vulkan-descriptor-and-buffer-management
	// 思路概要：
	//	1. 缓存 DescriptorSet，而不是每帧都反复创建销毁。
	//	2. 每一帧使用一个 buffer，记录所有 object 的 uniform matrix (mvp, jointMatrices), 然后在 vkCmdBindDescriptorSets 中使用 dynamicOffsets 区分每个物体。
	//
	outputDynamicOffsetCount	= _fillDynamicOffsets	(outputDynamicOffsets, outputDynamicOffsetCapacity, mvp, jointMatrices, jointMatrixCount);
	outputDescriptorSet			= _prepareDescriptorSet	(shader, uniformDatas, uniformDataCount, outputDescriptorSetLayout, outputDynamicOffsets, outputDynamicOffsetCount);
}


RenderTarget VulkanRender::_createRenderTarget(svkDevice& device, VkFormat colorFormat, VkFormat depthFormat, VkRenderPass renderPass, const int width, const int height)
{
	RenderTarget renderTarget;
	renderTarget.colorImage		= svkCreateAttachmentColorImage	(device, colorFormat, width, height);
	renderTarget.depthImage		= svkCreateAttachmentDepthImage	(device, depthFormat, width, height);
	VkImageView pickImageViews[] = { renderTarget.colorImage.imageView, renderTarget.depthImage.imageView };
	renderTarget.framebuffer	= svkCreateFrameBuffer			(device, renderPass, pickImageViews, 2, width, height); 
	return renderTarget;
}

//void VulkanRender::_destroyPickRenderTarget()
//{
//	vkDestroyFramebuffer(m_device.device, m_pickFramebuffer, NULL);
//	svkDestroyImage		(m_device, m_pickColorImage);
//	svkDestroyImage		(m_device, m_pickDepthImage);
//	vkDestroyRenderPass	(m_device.device, m_pickRenderPass, NULL);
//}


void VulkanRender::_destroyRenderTarget(svkDevice& device, RenderTarget& renderTarget)
{
	svkDestroyFrameBuffer(device, renderTarget.framebuffer);
	svkDestroyImage(device, renderTarget.colorImage);
	svkDestroyImage(device, renderTarget.depthImage);
	memclr(renderTarget);
}

void VulkanRender::_fillPushConst(VkCommandBuffer commandBuffer, svkPipeline& pipeline, void* _shader, void* pushConstBuffer, const int pushConstBufferSize)
{
	if (NULL == _shader)
		return;
	if (NULL == pushConstBuffer)
		return;

	svkShaderProgram* shader = static_cast<svkShaderProgram*>(_shader);
	for (int i = 0; i < shader->pushConstRangeCount; ++i)
	{
		VkPushConstantRange& range = shader->pushConstRanges[i];
		if (range.offset + range.size > pushConstBufferSize)
			continue;
		vkCmdPushConstants(commandBuffer, pipeline.layout, range.stageFlags, range.offset, range.size, static_cast<byte*>(pushConstBuffer) + range.offset);		
	}
}

void VulkanRender::_createMainRenderTarget()
{
	m_swapchain			= svkCreateSwapchain			(m_device, m_surface, { NULL }, 3, false);
	m_mainDepthImage	= svkCreateAttachmentDepthImage	(m_device, m_depthFormat, m_surface.width, m_surface.height);
	m_mainRenderPass	= svkCreateRenderPass			(m_device, m_swapchain.format, m_mainDepthImage.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	m_frameCount		= svkCreateFrames				(m_device, m_swapchain, m_mainDepthImage.imageView, m_mainRenderPass, m_surface.width, m_surface.height, m_frames, MAX_FRAME);
	m_frameIndex		= 0;
}

void VulkanRender::_destroyMainRenderTarget()
{
	svkDestroyFrames	(m_device, m_frames, m_frameCount);
	svkDestroyRenderPass(m_device, m_mainRenderPass);
	svkDestroyImage		(m_device, m_mainDepthImage);
	svkDestroySwapchain	(m_device, m_swapchain);
}

void VulkanRender::_fillDeviceInfo(svkDevice& device, DeviceInfo& info)
{
	// device info
    info.apiVersionMajor	= VK_VERSION_MAJOR(device.gpuProperties.apiVersion);
    info.apiVersionMinor	= VK_VERSION_MINOR(device.gpuProperties.apiVersion);
    info.apiVersionPatch	= VK_VERSION_PATCH(device.gpuProperties.apiVersion);
    info.driverVersion		= device.gpuProperties.driverVersion;
    info.vendorID			= device.gpuProperties.vendorID;
    info.deviceID			= device.gpuProperties.deviceID;
    info.deviceType			= (cat::DeviceInfo::DEVICE_TYPE)(device.gpuProperties.deviceType);
    scl::strcpy				(info.deviceName, 256, device.gpuProperties.deviceName);
    memcpy					(info.pipelineCacheUUID, device.gpuProperties.pipelineCacheUUID, sizeof(info.pipelineCacheUUID));

	// limits
	info.limits.maxImageDimension1D								= device.gpuProperties.limits.maxImageDimension1D;
	info.limits.maxImageDimension2D								= device.gpuProperties.limits.maxImageDimension2D;
	info.limits.maxImageDimension3D								= device.gpuProperties.limits.maxImageDimension3D;
	info.limits.maxImageDimensionCube							= device.gpuProperties.limits.maxImageDimensionCube;
	info.limits.maxImageArrayLayers								= device.gpuProperties.limits.maxImageArrayLayers;
	info.limits.maxTexelBufferElements							= device.gpuProperties.limits.maxTexelBufferElements;
	info.limits.maxUniformBufferRange							= device.gpuProperties.limits.maxUniformBufferRange;
	info.limits.maxStorageBufferRange							= device.gpuProperties.limits.maxStorageBufferRange;
	info.limits.maxPushConstantsSize							= device.gpuProperties.limits.maxPushConstantsSize;
	info.limits.maxMemoryAllocationCount						= device.gpuProperties.limits.maxMemoryAllocationCount;
	info.limits.maxSamplerAllocationCount						= device.gpuProperties.limits.maxSamplerAllocationCount;
	info.limits.bufferImageGranularity							= device.gpuProperties.limits.bufferImageGranularity;
	info.limits.sparseAddressSpaceSize							= device.gpuProperties.limits.sparseAddressSpaceSize;
	info.limits.maxBoundDescriptorSets							= device.gpuProperties.limits.maxBoundDescriptorSets;
	info.limits.maxPerStageDescriptorSamplers					= device.gpuProperties.limits.maxPerStageDescriptorSamplers;
	info.limits.maxPerStageDescriptorUniformBuffers				= device.gpuProperties.limits.maxPerStageDescriptorUniformBuffers;
	info.limits.maxPerStageDescriptorStorageBuffers				= device.gpuProperties.limits.maxPerStageDescriptorStorageBuffers;
	info.limits.maxPerStageDescriptorSampledImages				= device.gpuProperties.limits.maxPerStageDescriptorSampledImages;
	info.limits.maxPerStageDescriptorStorageImages				= device.gpuProperties.limits.maxPerStageDescriptorStorageImages;
	info.limits.maxPerStageDescriptorInputAttachments			= device.gpuProperties.limits.maxPerStageDescriptorInputAttachments;
	info.limits.maxPerStageResources							= device.gpuProperties.limits.maxPerStageResources;
	info.limits.maxDescriptorSetSamplers						= device.gpuProperties.limits.maxDescriptorSetSamplers;
	info.limits.maxDescriptorSetUniformBuffers					= device.gpuProperties.limits.maxDescriptorSetUniformBuffers;
	info.limits.maxDescriptorSetUniformBuffersDynamic			= device.gpuProperties.limits.maxDescriptorSetUniformBuffersDynamic;
	info.limits.maxDescriptorSetStorageBuffers					= device.gpuProperties.limits.maxDescriptorSetStorageBuffers;
	info.limits.maxDescriptorSetStorageBuffersDynamic			= device.gpuProperties.limits.maxDescriptorSetStorageBuffersDynamic;
	info.limits.maxDescriptorSetSampledImages					= device.gpuProperties.limits.maxDescriptorSetSampledImages;
	info.limits.maxDescriptorSetStorageImages					= device.gpuProperties.limits.maxDescriptorSetStorageImages;
	info.limits.maxDescriptorSetInputAttachments				= device.gpuProperties.limits.maxDescriptorSetInputAttachments;
	info.limits.maxVertexInputAttributes						= device.gpuProperties.limits.maxVertexInputAttributes;
	info.limits.maxVertexInputBindings							= device.gpuProperties.limits.maxVertexInputBindings;
	info.limits.maxVertexInputAttributeOffset					= device.gpuProperties.limits.maxVertexInputAttributeOffset;
	info.limits.maxVertexInputBindingStride						= device.gpuProperties.limits.maxVertexInputBindingStride;
	info.limits.maxVertexOutputComponents						= device.gpuProperties.limits.maxVertexOutputComponents;
	info.limits.maxTessellationGenerationLevel					= device.gpuProperties.limits.maxTessellationGenerationLevel;
	info.limits.maxTessellationPatchSize						= device.gpuProperties.limits.maxTessellationPatchSize;
	info.limits.maxTessellationControlPerVertexInputComponents	= device.gpuProperties.limits.maxTessellationControlPerVertexInputComponents;
	info.limits.maxTessellationControlPerVertexOutputComponents	= device.gpuProperties.limits.maxTessellationControlPerVertexOutputComponents;
	info.limits.maxTessellationControlPerPatchOutputComponents	= device.gpuProperties.limits.maxTessellationControlPerPatchOutputComponents;
	info.limits.maxTessellationControlTotalOutputComponents		= device.gpuProperties.limits.maxTessellationControlTotalOutputComponents;
	info.limits.maxTessellationEvaluationInputComponents		= device.gpuProperties.limits.maxTessellationEvaluationInputComponents;
	info.limits.maxTessellationEvaluationOutputComponents		= device.gpuProperties.limits.maxTessellationEvaluationOutputComponents;
	info.limits.maxGeometryShaderInvocations					= device.gpuProperties.limits.maxGeometryShaderInvocations;
	info.limits.maxGeometryInputComponents						= device.gpuProperties.limits.maxGeometryInputComponents;
	info.limits.maxGeometryOutputComponents						= device.gpuProperties.limits.maxGeometryOutputComponents;
	info.limits.maxGeometryOutputVertices						= device.gpuProperties.limits.maxGeometryOutputVertices;
	info.limits.maxGeometryTotalOutputComponents				= device.gpuProperties.limits.maxGeometryTotalOutputComponents;
	info.limits.maxFragmentInputComponents						= device.gpuProperties.limits.maxFragmentInputComponents;
	info.limits.maxFragmentOutputAttachments					= device.gpuProperties.limits.maxFragmentOutputAttachments;
	info.limits.maxFragmentDualSrcAttachments					= device.gpuProperties.limits.maxFragmentDualSrcAttachments;
	info.limits.maxFragmentCombinedOutputResources				= device.gpuProperties.limits.maxFragmentCombinedOutputResources;
	info.limits.maxComputeSharedMemorySize						= device.gpuProperties.limits.maxComputeSharedMemorySize;
	memcpy(info.limits.maxComputeWorkGroupCount,				device.gpuProperties.limits.maxComputeWorkGroupCount, sizeof(info.limits.maxComputeWorkGroupCount));
	info.limits.maxComputeWorkGroupInvocations					= device.gpuProperties.limits.maxComputeWorkGroupInvocations;
	memcpy(info.limits.maxComputeWorkGroupSize,					device.gpuProperties.limits.maxComputeWorkGroupSize, sizeof(info.limits.maxComputeWorkGroupSize));
	info.limits.subPixelPrecisionBits							= device.gpuProperties.limits.subPixelPrecisionBits;
	info.limits.subTexelPrecisionBits							= device.gpuProperties.limits.subTexelPrecisionBits;
	info.limits.mipmapPrecisionBits								= device.gpuProperties.limits.mipmapPrecisionBits;
	info.limits.maxDrawIndexedIndexValue						= device.gpuProperties.limits.maxDrawIndexedIndexValue;
	info.limits.maxDrawIndirectCount							= device.gpuProperties.limits.maxDrawIndirectCount;
	info.limits.maxSamplerLodBias								= device.gpuProperties.limits.maxSamplerLodBias;
	info.limits.maxSamplerAnisotropy							= device.gpuProperties.limits.maxSamplerAnisotropy;
	info.limits.maxViewports									= device.gpuProperties.limits.maxViewports;
	memcpy(info.limits.maxViewportDimensions,					device.gpuProperties.limits.maxViewportDimensions, sizeof(info.limits.maxViewportDimensions));
	memcpy(info.limits.viewportBoundsRange,						device.gpuProperties.limits.viewportBoundsRange, sizeof(info.limits.viewportBoundsRange));
	info.limits.viewportSubPixelBits							= device.gpuProperties.limits.viewportSubPixelBits;
	info.limits.minMemoryMapAlignment							= device.gpuProperties.limits.minMemoryMapAlignment;
	info.limits.minTexelBufferOffsetAlignment					= device.gpuProperties.limits.minTexelBufferOffsetAlignment;
	info.limits.minUniformBufferOffsetAlignment					= device.gpuProperties.limits.minUniformBufferOffsetAlignment;
	info.limits.minStorageBufferOffsetAlignment					= device.gpuProperties.limits.minStorageBufferOffsetAlignment;
	info.limits.minTexelOffset									= device.gpuProperties.limits.minTexelOffset;
	info.limits.maxTexelOffset									= device.gpuProperties.limits.maxTexelOffset;
	info.limits.minTexelGatherOffset							= device.gpuProperties.limits.minTexelGatherOffset;
	info.limits.maxTexelGatherOffset							= device.gpuProperties.limits.maxTexelGatherOffset;
	info.limits.minInterpolationOffset							= device.gpuProperties.limits.minInterpolationOffset;
	info.limits.maxInterpolationOffset							= device.gpuProperties.limits.maxInterpolationOffset;
	info.limits.subPixelInterpolationOffsetBits					= device.gpuProperties.limits.subPixelInterpolationOffsetBits;
	info.limits.maxFramebufferWidth								= device.gpuProperties.limits.maxFramebufferWidth;
	info.limits.maxFramebufferHeight							= device.gpuProperties.limits.maxFramebufferHeight;
	info.limits.maxFramebufferLayers							= device.gpuProperties.limits.maxFramebufferLayers;
	info.limits.framebufferColorSampleCounts					= device.gpuProperties.limits.framebufferColorSampleCounts;
	info.limits.framebufferDepthSampleCounts					= device.gpuProperties.limits.framebufferDepthSampleCounts;
	info.limits.framebufferStencilSampleCounts					= device.gpuProperties.limits.framebufferStencilSampleCounts;
	info.limits.framebufferNoAttachmentsSampleCounts			= device.gpuProperties.limits.framebufferNoAttachmentsSampleCounts;
	info.limits.maxColorAttachments								= device.gpuProperties.limits.maxColorAttachments;
	info.limits.sampledImageColorSampleCounts					= device.gpuProperties.limits.sampledImageColorSampleCounts;
	info.limits.sampledImageIntegerSampleCounts					= device.gpuProperties.limits.sampledImageIntegerSampleCounts;
	info.limits.sampledImageDepthSampleCounts					= device.gpuProperties.limits.sampledImageDepthSampleCounts;
	info.limits.sampledImageStencilSampleCounts					= device.gpuProperties.limits.sampledImageStencilSampleCounts;
	info.limits.storageImageSampleCounts						= device.gpuProperties.limits.storageImageSampleCounts;
	info.limits.maxSampleMaskWords								= device.gpuProperties.limits.maxSampleMaskWords;
	info.limits.timestampComputeAndGraphics						= device.gpuProperties.limits.timestampComputeAndGraphics;
	info.limits.timestampPeriod									= device.gpuProperties.limits.timestampPeriod;
	info.limits.maxClipDistances								= device.gpuProperties.limits.maxClipDistances;
	info.limits.maxCullDistances								= device.gpuProperties.limits.maxCullDistances;
	info.limits.maxCombinedClipAndCullDistances					= device.gpuProperties.limits.maxCombinedClipAndCullDistances;
	info.limits.discreteQueuePriorities							= device.gpuProperties.limits.discreteQueuePriorities;
	memcpy(info.limits.pointSizeRange,							device.gpuProperties.limits.pointSizeRange, sizeof(info.limits.pointSizeRange));
	memcpy(info.limits.lineWidthRange,							device.gpuProperties.limits.lineWidthRange, sizeof(info.limits.lineWidthRange));
	info.limits.pointSizeGranularity							= device.gpuProperties.limits.pointSizeGranularity;
	info.limits.lineWidthGranularity							= device.gpuProperties.limits.lineWidthGranularity;
	info.limits.strictLines										= device.gpuProperties.limits.strictLines;
	info.limits.standardSampleLocations							= device.gpuProperties.limits.standardSampleLocations;
	info.limits.optimalBufferCopyOffsetAlignment				= device.gpuProperties.limits.optimalBufferCopyOffsetAlignment;
	info.limits.optimalBufferCopyRowPitchAlignment				= device.gpuProperties.limits.optimalBufferCopyRowPitchAlignment;
	info.limits.nonCoherentAtomSize								= device.gpuProperties.limits.nonCoherentAtomSize;
}

//
// 2022.10.14 尝试拆解该方法为以下几个部分供 client 更加灵活的调用：
//		1. beginCommandBuffer
//		2. bindUniform
//		3. setVertex
//		4. draw
//		5. endCommandBuffer
//		但是由于 uniform 和 vertex 都和 pipeline 偶合，所以 2,3,4 很难拆开。 :(
//
void VulkanRender::draw2(
	void*				texture,
	void**				vertexBuffers, 
	const int			primitiveType,
	void*				indexBuffer, 
	const int			indexCount,
	const int			indexComponentType,
	const int			indexOffset,
	int					attrCount,
	const VertexAttr*	attrs,
	void*				shader,
	const scl::matrix&	mvp,
	const scl::matrix*	jointMatrices,
	const int			jointMatrixCount,
	void*				pushConstBuffer,
	const int			pushConstBufferSize
)
{
	if (_minimized())
		return;

	if (NULL == m_drawContext.renderPass)
		return;

	//VkCommandBuffer cmd_buf = m_swapchain.commandBuffers[m_frameIndex];
	bool			useBindCommandBuffer	= m_bindCommandBuffer != NULL;
	VkCommandBuffer	cmd_buf					= useBindCommandBuffer ? m_bindCommandBuffer : m_drawContext.commandAllocator->alloc();

	if (!useBindCommandBuffer)
		svkBeginSecondaryCommandBuffer(cmd_buf, m_drawContext.renderPass, m_drawContext.framebuffer);

	DescriptorSet			descriptorSet								= { 0 };
	VkDescriptorSetLayout	descriptorSetLayout							= NULL;
	const int				MAX_DYNAMIC_OFFSET_COUNT					= 2;
	uint32_t				dynamicOffsets[MAX_DYNAMIC_OFFSET_COUNT]	= { 0 };
	uint32_t				dynamicOffsetCount							= 0;
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
		dynamicOffsetCount, 
		dynamicOffsets);

	svkCmdBindVertexBuffers(cmd_buf, 0, vertexBuffers, attrCount);                

	svkBuffer* svkIndexBuffer = static_cast<svkBuffer*>(indexBuffer);

	vkCmdBindIndexBuffer(cmd_buf, svkIndexBuffer->buffer, indexOffset, _toVkIndexType(indexComponentType));

	_fillPushConst(cmd_buf, *pipeline, shader, pushConstBuffer, pushConstBufferSize);

	svkCmdSetViewPortByGLParams(cmd_buf, 0, 0, m_drawContext.width, m_drawContext.height);

	svkCmdSetScissor(cmd_buf, m_drawContext.width, m_drawContext.height);

	vkCmdDrawIndexed(
		cmd_buf, 
		indexCount,
		1,
		0,
		0,
		0);

	if (!useBindCommandBuffer)
		svkEndCommandBuffer(cmd_buf);
}


void VulkanRender::updateMVP(const scl::matrix& mvp)
{
	assert(false);
	//for (int i = 0; i < MAX_FRAME; ++i)
	//{
	//	memcpy(m_frameUniformBuffersMapped[i], (const void *)&mvp, sizeof(scl::matrix));
	//}
}

void VulkanRender::waitIdle()
{
	if (NULL != m_device.device)
		svkDeviceWaitIdle(m_device);
}

void VulkanRender::recreateSwapchain()
{
	svkRefreshSurfaceSize(m_device, m_surface);
	if (_minimized())
		return;

	m_onSurfaceResize(m_surface.width, m_surface.height);

	_destroyMainRenderTarget();
	_createMainRenderTarget();

	//_destroyPickRenderTarget();
	//vkDestroyRenderPass(m_device.device, m_pickRenderPass, NULL);
	//m_pickRenderPass = svkCreateRenderPass(m_device, m_colorFormat, m_depthFormat, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	//svkDestroyBuffer(m_device, m_pickPassImageCPUBuffer);
	//m_pickPassImageCPUBuffer = svkCreateBuffer(m_device, VK_BUFFER_USAGE_TRANSFER_DST_BIT, m_surface.width * m_surface.height * 4);

	_destroyRenderTarget(m_device, m_pickRenderTarget);
	m_pickRenderTarget = _createRenderTarget(m_device, m_colorFormat, m_depthFormat, m_pickRenderPass, m_surface.width, m_surface.height);
}

void VulkanRender::recreateSurface()
{
	svkDestroySurface(m_inst, m_device, m_surface);
	m_surface = svkCreateSurface(m_inst, m_device, m_windowInstance, m_windowHandle);
}

void VulkanRender::releaseIMGUI()
{
	waitIdle();
	ImGui_ImplVulkan_Shutdown();
	svkDestroyDescriptorPool(m_device, m_IMGUIDescriptorPool);
}

void VulkanRender::drawIMGUI(ImDrawData* draw_data)
{
	if (_minimized())
		return;

	bool useBindCommandBuffer = m_bindCommandBuffer != NULL;
	VkCommandBuffer cb = useBindCommandBuffer ? m_bindCommandBuffer : m_drawContext.commandAllocator->alloc();

	if (!useBindCommandBuffer)
		svkBeginSecondaryCommandBuffer(cb, m_drawContext.renderPass, m_drawContext.framebuffer);

	ImGui_ImplVulkan_RenderDrawData(draw_data, cb);

	if (!useBindCommandBuffer)
	{
		VkResult err = svkEndCommandBuffer(cb);
		assert(!err);
	}
}

void VulkanRender::bindCommandBuffer()
{
	assert(NULL == m_bindCommandBuffer);
	m_bindCommandBuffer = m_drawContext.commandAllocator->alloc();
	svkBeginSecondaryCommandBuffer(m_bindCommandBuffer, m_drawContext.renderPass, m_drawContext.framebuffer);
}

void VulkanRender::unbindCommandBuffer()
{
	svkEndCommandBuffer(m_bindCommandBuffer);
	m_bindCommandBuffer = NULL;
}

bool VulkanRender::_minimized()
{
	return getDeviceWidth() == 0 || getDeviceHeight() == 0;
}


VkFormat _attrToVkFormat(const VertexAttr& attr)
{
	VkFormat result = VK_FORMAT_UNDEFINED;
	switch(attr.dataType)
	{
	case ELEM_TYPE_INT8:
		{
			if (attr.normalize)
			{
				switch (attr.size)
				{
				case 1: result = VK_FORMAT_R8_SNORM;		break;
				case 2: result = VK_FORMAT_R8G8_SNORM;		break;
				case 3: result = VK_FORMAT_R8G8B8_SNORM;	break;
				case 4: result = VK_FORMAT_R8G8B8A8_SNORM;	break;
				};
			}
			else
			{
				switch (attr.size)
				{
				case 1: result = VK_FORMAT_R8_SINT;			break;
				case 2: result = VK_FORMAT_R8G8_SINT;		break;
				case 3: result = VK_FORMAT_R8G8B8_SINT;		break;
				case 4: result = VK_FORMAT_R8G8B8A8_SINT;	break;
				};
			}
		}
		break;
	case ELEM_TYPE_UINT8:
		{
			if (attr.normalize)
			{
				switch (attr.size)
				{
				case 1: result = VK_FORMAT_R8_UNORM;		break;
				case 2: result = VK_FORMAT_R8G8_UNORM;		break;
				case 3: result = VK_FORMAT_R8G8B8_UNORM;	break;
				case 4: result = VK_FORMAT_R8G8B8A8_UNORM;	break;
				};
			}
			else
			{
				switch (attr.size)
				{
				case 1: result = VK_FORMAT_R8_UINT;			break;
				case 2: result = VK_FORMAT_R8G8_UINT;		break;
				case 3: result = VK_FORMAT_R8G8B8_UINT;		break;
				case 4: result = VK_FORMAT_R8G8B8A8_UINT;	break;
				};
			}
		}
		break;
	case ELEM_TYPE_INT16:
		{
			if (attr.normalize)
			{
				switch (attr.size)
				{
				case 1: result = VK_FORMAT_R16_SNORM ;			break;
				case 2: result = VK_FORMAT_R16G16_SNORM;		break;
				case 3: result = VK_FORMAT_R16G16B16_SNORM;		break;
				case 4: result = VK_FORMAT_R16G16B16A16_SNORM;	break;
				};
			}
			else
			{
				switch (attr.size)
				{
				case 1: result = VK_FORMAT_R16_SINT;			break;
				case 2: result = VK_FORMAT_R16G16_SINT;			break;
				case 3: result = VK_FORMAT_R16G16B16_SINT;		break;
				case 4: result = VK_FORMAT_R16G16B16A16_SINT;	break;
				};
			}
		}
		break;
	case ELEM_TYPE_UINT16:
		{
			if (attr.normalize)
			{
				switch (attr.size)
				{
				case 1: result = VK_FORMAT_R16_UNORM;			break;
				case 2: result = VK_FORMAT_R16G16_UNORM;		break;
				case 3: result = VK_FORMAT_R16G16B16_UNORM;		break;
				case 4: result = VK_FORMAT_R16G16B16A16_UNORM;	break;
				};
			}
			else
			{
				switch (attr.size)
				{
				case 1: result = VK_FORMAT_R16_UINT;			break;
				case 2: result = VK_FORMAT_R16G16_UINT;			break;
				case 3: result = VK_FORMAT_R16G16B16_UINT;		break;
				case 4: result = VK_FORMAT_R16G16B16A16_UINT;	break;
				
				// GLSL  don't support 16-bit vec4, so use float instead.
				//case 1: result = VK_FORMAT_R16_SFLOAT;			break;
				//case 2: result = VK_FORMAT_R16G16_SFLOAT;		break;
				//case 3: result = VK_FORMAT_R16G16B16_SFLOAT;	break;
				//case 4: result = VK_FORMAT_R16G16B16A16_SFLOAT;	break;
				};
			}
		}
		break;
	case ELEM_TYPE_UINT32:
		{
			assert(!attr.normalize);
			switch (attr.size)
			{
			case 1: result = VK_FORMAT_R32_UINT;			break;
			case 2: result = VK_FORMAT_R32G32_UINT;			break;
			case 3: result = VK_FORMAT_R32G32B32_UINT;		break;
			case 4: result = VK_FORMAT_R32G32B32A32_UINT;	break;
			};
		}
		break;
	case ELEM_TYPE_FLOAT:
		{
			assert(!attr.normalize);
			switch (attr.size)
			{
			case 1: result = VK_FORMAT_R32_SFLOAT;			break;
			case 2: result = VK_FORMAT_R32G32_SFLOAT;		break;
			case 3: result = VK_FORMAT_R32G32B32_SFLOAT;	break;
			case 4: result = VK_FORMAT_R32G32B32A32_SFLOAT;	break;
			};
		}
		break;
	case ELEM_TYPE_INT32:
		{
			assert(!attr.normalize);
			switch (attr.size)
			{
			case 1: result = VK_FORMAT_R32_SINT;			break;
			case 2: result = VK_FORMAT_R32G32_SINT;			break;
			case 3: result = VK_FORMAT_R32G32B32_SINT;		break;
			case 4: result = VK_FORMAT_R32G32B32A32_SINT;	break;
			};
		}
		break;
	default:
		assert(false);
		break;
	};
	return result;
}

VkPipelineVertexInputStateCreateInfo _buildVulkanVertexInput(const VertexAttr* attrs, const int attrCount, void** vertexBuffers, VkVertexInputBindingDescription* viBinds, VkVertexInputAttributeDescription* viAttrs)
{
	VkPipelineVertexInputStateCreateInfo viCreateInfo;
	memclr(viCreateInfo);
	viCreateInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	// 注意！这里buffer的index和attr的index是分开的
	// 可以理解为一个buffer带n个attr。
	// 那为什么传进来的时候不是类似下面这种结构呢？
	//  struct Buffer 
	//	{
	//		int stride;
	//		Attr attrs[n];
	//	}
	// 原因在于，在gltf中，数据不是以buffer为主进行存放的，而是以属性为主进行存放的
	//
	//	"primitives" : [
	//	{
	//		"attributes" : {
	//				"NORMAL"		: 2,	// 这里的 id 对应 accessor ，accessor 又对应 buffer view，buffer view 会指向实际的 buffer, 
	//				"POSITION"		: 1,	// 这里虽然 accessor 和 buffer view 都不同，但是指向的 buffer 可能是同一个
	//				"TANGENT"		: 3,	// 也可能是 accessor 不同，但是 buffer view 相同，offset 不同。
	//				"TEXCOORD_0"	: 4		// nvidia 里面关于 vulkan 的推荐是，尽量少创建 gpu buffer，而是多用 offset
	//		},								// 目前 Primitive 类读取 gltf 文件的时候，没有在这里进行多个 attr 整合到一个 buffer 上的逻辑，所以在 bind vertex 的时候，需要处理一下。
	//										// 更合理的做法是，在 Primitive 类中处理。
	//		...
	//	} 	]

	// VkVertexInputBindingDescription		是用来描述一个 buffer 的绑定信息的。
	//										所以只有在存在多个buffer的时候，我们才需要创建多个 viBinds，这也是下面 binding 变量的逻辑。
	// VkVertexInputAttributeDescription	是用来描述顶点属性，以及该属性在shader中的变量的location。
	//										每个顶点属性都要对应创建一个 viAttrs
	//
	// 创建的多个 bind des 和 attr des 会通过 VkPipelineVertexInputStateCreateInfo 返回。

	int			binding		= -1;
	const void*	prevBuffer	= NULL;
	int			viAttrCount	= 0;
	for (int i = 0; i < attrCount; ++i)
	{
		const VertexAttr&					attr	= attrs[i];
		if (attr.location < 0)
			continue;

		const void*							buffer	= vertexBuffers[i];
		VkVertexInputAttributeDescription&	viAttr	= viAttrs[viAttrCount];
		if (NULL == buffer)
			buffer = vertexBuffers[0];

		if (buffer != prevBuffer)
		{
			++binding;
			VkVertexInputBindingDescription& viBind	= viBinds[binding];
			viBind.binding							= binding;
			viBind.stride							= attr.stride;
			viBind.inputRate						= VK_VERTEX_INPUT_RATE_VERTEX;
			prevBuffer								= buffer;
		}

		viAttr.location								= attr.location;
		viAttr.binding								= binding;
		viAttr.format								= _attrToVkFormat(attr); 
		viAttr.offset								= uint32_t((uintptr_t(attr.offset)));
		++viAttrCount;
	}

	viCreateInfo.vertexBindingDescriptionCount		= binding + 1;
	viCreateInfo.pVertexBindingDescriptions			= viBinds;
	viCreateInfo.vertexAttributeDescriptionCount	= viAttrCount;
	viCreateInfo.pVertexAttributeDescriptions		= viAttrs;

	return viCreateInfo;
}

VkPrimitiveTopology _toVulkanPrimitiveTopology(PRIMITIVE_TYPE type)
{
	switch (type)
	{
	case PRIMITIVE_TYPE_POINTS			: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case PRIMITIVE_TYPE_LINES			: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case PRIMITIVE_TYPE_LINE_LOOP		: assert(false); return VK_PRIMITIVE_TOPOLOGY_LINE_LIST; // vulkan doesn't support line loop
	case PRIMITIVE_TYPE_LINE_STRIP		: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case PRIMITIVE_TYPE_TRIANGLES		: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case PRIMITIVE_TYPE_TRIANGLE_STRIP	: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case PRIMITIVE_TYPE_TRIANGLE_FAN	: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
	default: assert(false); break;
	};
	return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
}

} //namespace cat


