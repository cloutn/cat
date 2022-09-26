#include "./vulkanRender.h"
#include "./descriptorAllocator.h"
#include "./commandAllocator.h"


#include "libimg/image.h"

#include "scl/stringdef.h"
#include "scl/log.h"
#include "scl/math.h"

#include "xxhash.h"

#include "imgui/imgui.h"
#include "imgui_impl_vulkan.h"

#include <assert.h>

#define memclr(s) memset(&s, 0, sizeof(s))

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
	m_isInit					(false),
	m_pickRenderPass			(NULL),
	m_minimized					(false),
	//m_prepared					(false),
	m_frameIndex				(-1),
	m_prevFrameIndex			(-1),
	m_matrixChanged				(false),
	m_scale						(1.0f),
	m_frameUniformBufferOffset	(0),
	m_windowInstance			(NULL),
	m_windowHandle				(NULL),
	m_IMGUIDescriptorPool		(NULL),
	m_bindCommandBuffer			(NULL)
	//m_clearColor				(0)
	
{
	memclr(m_device);
	memclr(m_surface);
	memclr(m_swapchain);
	memclr(m_device);
	memclr(m_surface);
	memclr(m_frameUniforms);
	memclr(m_frameUniformBuffersMapped);
	memclr(m_clearColor);
	memclr(m_frames);
	memclr(m_pickColorImage);
	memclr(m_pickDepthImage);
}


bool VulkanRender::init(void* hInstance, void* hwnd, const uint32 clearColor)
{
	if (is_init())
		return false;

	// init hash tables
	m_pipelines.init			(MAX_PIPELINE_COUNT * MAX_CONFLICT);
	m_descriptorAllocators.init	(MAX_DESCRITOR_ALLOCATOR_COUNT * MAX_CONFLICT);
	m_descriptorSetCache.init	(MAX_DESCRIPTOR_SET_CACHE_SIZE * MAX_CONFLICT);

	m_windowInstance	= hInstance;
	m_windowHandle		= hwnd;

	m_inst				= svkCreateInstance	(ENABLE_VALIDATION_LAYER);
	m_device			= svkCreateDevice	(m_inst);
	m_surface			= svkCreateSurface	(m_inst, m_device, hInstance, hwnd);
	argb_to_float(clearColor, m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);

	//svkRefreshSurfaceSize(m_device, m_surface);
	//m_minimized = m_surface.width == 0 && m_surface.height == 0;
	//if (m_minimized)
	//{
	//	//m_prepared = false;
	//	return false;
	//}

	m_swapchain			= svkCreateSwapchain			(m_device, m_surface, m_swapchain);
	m_mainDepthImage	= svkCreateAttachmentDepthImage	(m_device, VK_FORMAT_D16_UNORM, m_surface.width, m_surface.height);
	m_mainRenderPass	= svkCreateRenderPass			(m_device, m_swapchain.format, m_mainDepthImage.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	m_frameCount		= svkCreateFrames				(m_device, m_swapchain, m_mainDepthImage.imageView, m_mainRenderPass, m_surface.width, m_surface.height, m_frames, MAX_FRAME);
	m_frameIndex		= 0;

	// 3D picking
	m_pickColorImage	= svkCreateAttachmentColorImage	(m_device, VK_FORMAT_R8G8B8A8_UNORM, m_surface.width, m_surface.height);
	m_pickDepthImage	= svkCreateAttachmentDepthImage	(m_device, VK_FORMAT_D16_UNORM, m_surface.width, m_surface.height);
	m_pickRenderPass	= svkCreateRenderPass			(m_device, m_swapchain.format, m_mainDepthImage.format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


	// descriptor set
	//const int DEMO_TEXTURE_COUNT = 1;
	//m_descriptorCreator = svkCreateDescriptorCreator(m_device, m_swapchain.imageCount, 1, DEMO_TEXTURE_COUNT);

	// mvp matrix
	//scl::matrix mvp =
	//{ 
	//	{1.61543f,	0.92307f,	-0.63852f, -0.63725f,
	//	0,			-2.07017f,	-0.51553f, -0.5145f,
	//	-1.79413f, 0.83113f,	-0.57493f, -0.57378f,
	//	 0,		0,			5.64243f,	5.83095f},
	//};

	//scl::matrix mvp =
	//{ 
	//	{1.449f,	0,			0,		0,
	//	0,			2.414f,		0,		0,
	//	0,			0,			-1,		-1,
	//	0.056f,		-0.833f,	-0.2,	0},
	//};

	//scl::matrix mvp =
	//{ 
	//	{1.449f,	0,			0,		0.056f,
	//	0,			2.414f,		0,		-0.833f,
	//	0,			0,			-1,		-0.2,
	//	0,			0,			-1,		0},
	//};

	scl::matrix mvp = scl::matrix::identity();

	for (int i = 0; i < static_cast<int>(m_swapchain.imageCount); ++i)
	{
		if (NULL != m_frameUniforms[i].buffer)
		{
			svkUnmapBuffer	(m_device, m_frameUniforms[i]);
			svkDestroyBuffer(m_device, m_frameUniforms[i]);
		}

		int minUniformBufferOffset		= static_cast<int>(m_device.gpuProperties.limits.minUniformBufferOffsetAlignment);
		int maxBytesPerFrace			= minUniformBufferOffset * MAX_OBJECT_PER_FRAME * MAX_MATRIX_PER_FRAME;
		m_frameUniforms[i]				= svkCreateUniformBuffer(m_device, NULL, maxBytesPerFrace);
		m_frameUniformBuffersMapped[i]	= svkMapBuffer			(m_device, m_frameUniforms[i]);
	}

	//m_prepared = true;

	for (int i = 0; i < static_cast<int>(m_swapchain.imageCount); ++i)
	{
		m_commandAllocator[i] = new CommandAllocator();
		m_commandAllocator[i]->init(m_device);
	}

	m_isInit = true;

	return true;
}


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

	ImGui_ImplVulkan_Init		(&init_info, m_mainRenderPass);

	// Upload Fonts
	VkCommandBuffer textureCommandBuffer		= svkCreateCommandBuffer(m_device);
	svkBeginCommandBuffer						(textureCommandBuffer);;
	ImGui_ImplVulkan_CreateFontsTexture			(textureCommandBuffer);
	svkEndCommandBuffer							(m_device, textureCommandBuffer);
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

	svkDestroyImage		(m_device, m_pickColorImage);
	svkDestroyImage		(m_device, m_pickDepthImage);
	vkDestroyRenderPass	(m_device.device, m_pickRenderPass, NULL);

	vkDestroyRenderPass	(m_device.device, m_mainRenderPass, NULL);
	svkDestroyFrames	(m_device, m_frames, m_frameCount);
	svkDestroyImage		(m_device, m_mainDepthImage);
	svkDestroySwapchain	(m_device, m_swapchain, true);
	svkDestroySurface	(m_inst, m_device, m_surface);
	svkDestroyDevice	(m_device);
	vkDestroyInstance	(m_inst, NULL);
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

void VulkanRender::copyVertexBuffer(const void* data, void* vertexBuffer, const int sizeInByte)
{
	if (NULL == vertexBuffer)
		return;
	svkBuffer* buf = static_cast<svkBuffer*>(vertexBuffer);
	if (NULL != buf->buffer)
	{
		svkCopyBuffer(m_device, *buf, data, sizeInByte);
	}
	else
	{
		*buf = svkCreateVertexBuffer(m_device, data, sizeInByte);
	}
}

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


void VulkanRender::copyIndexBuffer(const void* data, void* indexBuffer, const int sizeInByte)
{
	if (NULL == indexBuffer)
		return;
	svkBuffer* buf = static_cast<svkBuffer*>(indexBuffer);
	if (NULL != buf->buffer)
	{
		assert(false);
		return;
	}
	*buf = svkCreateIndexBuffer(m_device, data, sizeInByte);
}


void* VulkanRender::createTexture(const char* const filename, int* width, int* height, int* pitch, PIXEL* pixel)
{
	//char s[256] = { 0 };
	//scl::wchar_to_ansi(s, 256, filename, static_cast<int>(wcslen(filename)), scl::Encoding_UTF8);

	//创建纹理   
	svkTexture* tex = new svkTexture; 
	*tex = svkCreateTexture(m_device, filename, NULL); 

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

int VulkanRender::getDeviceWidth()
{
	return m_surface.width;
}

int VulkanRender::getDeviceHeight()
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

	svkQueueSubmitFrame(m_device, m_frames, m_frameIndex, m_prevFrameIndex);
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

	m_prevFrameIndex = m_frameIndex;
	m_frameIndex = svkAcquireNextImage(m_device, m_swapchain, m_frames, m_frameIndex, this, presentCallback);
	m_commandAllocator[m_frameIndex]->reset(m_device);
	m_frameUniformBufferOffset = 0;
}

void VulkanRender::endDraw()
{
	if (_minimized())
		return;

	VkResult err;

	VkCommandBuffer& primaryCb = m_frames[m_frameIndex].commandBuffer;
	svkBeginCommandBuffer(primaryCb);
	svkCmdBeginRenderPass(primaryCb, m_clearColor[1], m_clearColor[2], m_clearColor[3], m_clearColor[0], 1.0f, 0, m_mainRenderPass, m_frames[m_frameIndex].framebuffer, m_surface.width, m_surface.height, true);
	
	CommandAllocator* commandAllocator = m_commandAllocator[m_frameIndex];
	vkCmdExecuteCommands(primaryCb, commandAllocator->getAllocCount(), commandAllocator->getAllocArray());

	vkCmdEndRenderPass(primaryCb);
	err = vkEndCommandBuffer(primaryCb);
	assert(!err);
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
	uniformDatas[idx].data[0].buffer.buffer		= m_frameUniforms[m_frameIndex].buffer;
	uniformDatas[idx].data[0].buffer.bufferSize	= sizeof(mvp);
	uniformDatas[idx].dataCount = 1;
	uniformDatas[idx].binding = 0;
	++idx;

	// [1]
	if (jointMatrixCount > 0)
	{
		uniformDatas[idx].dataCount					= min(jointMatrixCount, MAX_INFO_COUNT);	
		for (int i = 0; i < uniformDatas[idx].dataCount; ++i)
		{
			uniformDatas[idx].data[i].buffer.buffer	= m_frameUniforms[m_frameIndex].buffer;
			uniformDatas[idx].data[i].buffer.bufferSize = sizeof(scl::matrix) * jointMatrixCount;
		}
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

void VulkanRender::_prepareDescriptorSet(
	void*					shader, 
	svkDescriptorData*		uniformDatas,
	const int				uniformDataCount,
	DescriptorSet&			descriptorSet,
	DescriptorAllocator*&	descriptorAllocator)
{
	svkShaderProgram* shaderProgram = static_cast<svkShaderProgram*>(shader);

	VkDescriptorSetLayoutBinding*	uniformBinds		= shaderProgram->layoutBinds;
	int								uniformBindCount	= shaderProgram->layoutBindCount;
	if (NULL != shaderProgram->layoutBinds && shaderProgram->layoutBindCount > 0)
	{
		int			uniformBindHash = XXH32(uniformBinds, sizeof(VkDescriptorSetLayoutBinding) * uniformBindCount, 0);	
		const int	findIndex		= m_descriptorAllocators.find_index(uniformBindHash);
		if (findIndex != -1)
		{
			descriptorAllocator		= m_descriptorAllocators.get_value(findIndex);
		}
		else
		{
			descriptorAllocator		= new DescriptorAllocator();
			descriptorAllocator->init(m_device, uniformBinds, uniformBindCount);
			m_descriptorAllocators.add(uniformBindHash, descriptorAllocator);
		}

		UniformDataKey dataKey;
		dataKey.init(uniformBinds, uniformBindCount, uniformDatas, m_frameUniforms[m_frameIndex]);
		int cacheIndex = m_descriptorSetCache.find_index(dataKey);
		if (cacheIndex != -1)
		{
			descriptorSet = m_descriptorSetCache.get_value(cacheIndex);
		}
		else
		{
			descriptorSet = descriptorAllocator->alloc();
			svkUpdateDescriptorSet(m_device, descriptorSet.set, uniformBinds, uniformBindCount, uniformDatas, uniformDataCount);
			m_descriptorSetCache.add(dataKey, descriptorSet);
		}
	}
}


void VulkanRender::_preparePipeline(
	const int				primitiveType,
	const VertexAttr*		attrs,
	const int				attrCount,
	void**					vertexBuffers,
	DescriptorAllocator*	descriptorAllocator,
	void*					shader,
	svkPipeline*&			pipeline)
{
	VkPrimitiveTopology	vkTopology  = _toVulkanPrimitiveTopology(static_cast<PRIMITIVE_TYPE>(primitiveType));
	PipelineKey			pipelineKey(attrs, shader, vkTopology);
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
		*pipeline	= svkCreatePipelineEx(m_device, descriptorAllocator->getLayout(), m_mainRenderPass, vkTopology, viCreateInfo, *shaderProgram, NULL);
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

	memcpy((byte*)(m_frameUniformBuffersMapped[m_frameIndex]) + m_frameUniformBufferOffset, &mvp, sizeof(mvp));
	dynamicOffsets[0]			= m_frameUniformBufferOffset;
	m_frameUniformBufferOffset	+= _alignUniformBufferOffset(sizeof(mvp));
	++dynamicOffsetCount;

	if (jointMatrixCount > 0 && NULL != jointMatrices)
	{
		memcpy((byte*)(m_frameUniformBuffersMapped[m_frameIndex]) + m_frameUniformBufferOffset, jointMatrices, sizeof(scl::matrix) * jointMatrixCount);
		dynamicOffsets[1]			= m_frameUniformBufferOffset;
		m_frameUniformBufferOffset	+= _alignUniformBufferOffset(sizeof(scl::matrix) * jointMatrixCount);
		++dynamicOffsetCount;
	}
	return dynamicOffsetCount;
}

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
	const int			jointMatrixCount)
{
	if (_minimized())
		return;

	//VkCommandBuffer cmd_buf = m_swapchain.commandBuffers[m_frameIndex];
	bool			useBindCommandBuffer	= m_bindCommandBuffer != NULL;
	VkCommandBuffer	cmd_buf					= useBindCommandBuffer ? m_bindCommandBuffer : m_commandAllocator[m_frameIndex]->alloc();

	if (!useBindCommandBuffer)
		svkBeginSecondaryCommandBuffer(cmd_buf, m_mainRenderPass, m_frames[m_frameIndex].framebuffer);

	// uniform 数据分为两个部分，
	//		一个是声明信息，就是uniform bind
	//		一个是具体数据信息，就是 uniform data
	//		具体数据要和前面声明的类型对应上。

	svkDescriptorData uniformDatas[3];
	int uniformDataCount = _fillUniformData(mvp, texture, jointMatrixCount, uniformDatas, countof(uniformDatas));

	// 这里使用 dynamic offset 进行优化
	// 具体优化思路可以参考 arm 的文章 ：https://community.arm.com/developer/tools-software/graphics/b/blog/posts/vulkan-descriptor-and-buffer-management
	// 思路概要：
	//	1. 缓存 DescriptorSet，而不是每帧都反复创建销毁。
	//	2. 每一帧使用一个 buffer，记录所有 object 的 uniform matrix (mvp, jointMatrices), 然后在 vkCmdBindDescriptorSets 中使用 dynamicOffsets 区分每个物体。
	//
	const int	MAX_DYNAMIC_OFFSET_COUNT					= 2;
	uint32_t	dynamicOffsets[MAX_DYNAMIC_OFFSET_COUNT]	= { 0 };
	uint32_t	dynamicOffsetCount							= _fillDynamicOffsets(dynamicOffsets, countof(dynamicOffsets), mvp, jointMatrices, jointMatrixCount);

	DescriptorSet			descriptorSet;
	DescriptorAllocator*	desAllocator = NULL;
	_prepareDescriptorSet(shader, uniformDatas, uniformDataCount, descriptorSet, desAllocator);

	svkPipeline* pipeline = NULL;
	_preparePipeline(primitiveType, attrs, attrCount, vertexBuffers, desAllocator, shader, pipeline);

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

	svkCmdSetViewPortByGLParams(cmd_buf, 0, 0, m_surface.width, m_surface.height);

	svkCmdSetScissor(cmd_buf, m_surface.width, m_surface.height);

	vkCmdDrawIndexed(
		cmd_buf, 
		indexCount,
		1,
		0,
		0,
		0);

	if (!useBindCommandBuffer)
		vkEndCommandBuffer(cmd_buf);
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
		vkDeviceWaitIdle(m_device.device);
}

void VulkanRender::recreateSwapchain()
{
	svkRefreshSurfaceSize(m_device, m_surface);
	if (_minimized())
		return;

	svkDestroyFrames	(m_device, m_frames, m_frameCount);
	vkDestroyRenderPass	(m_device.device, m_mainRenderPass, NULL);
	svkDestroyImage		(m_device, m_mainDepthImage);
	svkDestroySwapchain	(m_device, m_swapchain);

	svkSwapchain oldswapchain = { NULL };
	m_swapchain			= svkCreateSwapchain			(m_device, m_surface, oldswapchain, 3, false);
	m_mainDepthImage	= svkCreateAttachmentDepthImage	(m_device, VK_FORMAT_D16_UNORM, m_surface.width, m_surface.height);
	m_mainRenderPass	= svkCreateRenderPass			(m_device, m_swapchain.format, m_mainDepthImage.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	m_frameCount		= svkCreateFrames				(m_device, m_swapchain, m_mainDepthImage.imageView, m_mainRenderPass, m_surface.width, m_surface.height, m_frames, MAX_FRAME);
	m_frameIndex		= 0;
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
	vkDestroyDescriptorPool(m_device.device, m_IMGUIDescriptorPool, NULL);
}

void VulkanRender::drawIMGUI(ImDrawData* draw_data)
{
	if (_minimized())
		return;

	bool useBindCommandBuffer = m_bindCommandBuffer != NULL;
	VkCommandBuffer cb = useBindCommandBuffer ? m_bindCommandBuffer : m_commandAllocator[m_frameIndex]->alloc();

	if (!useBindCommandBuffer)
		svkBeginSecondaryCommandBuffer(cb, m_mainRenderPass, m_frames[m_frameIndex].framebuffer);

	ImGui_ImplVulkan_RenderDrawData(draw_data, cb);

	if (!useBindCommandBuffer)
	{
		VkResult err = vkEndCommandBuffer(cb);
		assert(!err);
	}
}

void VulkanRender::bindCommandBuffer()
{
	assert(NULL == m_bindCommandBuffer);
	m_bindCommandBuffer = m_commandAllocator[m_frameIndex]->alloc();
	svkBeginSecondaryCommandBuffer(m_bindCommandBuffer, m_mainRenderPass, m_frames[m_frameIndex].framebuffer);
}

void VulkanRender::unbindCommandBuffer()
{
	vkEndCommandBuffer(m_bindCommandBuffer);
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

	int			binding		= -1;
	const void*	prevBuffer	= NULL;
	int			viAttrCount	= 0;
	for (int i = 0; i < attrCount; ++i)
	{
		const VertexAttr&					attr	= attrs[i];
		if (attr.index < 0)
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

		viAttr.location								= attr.index;
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


