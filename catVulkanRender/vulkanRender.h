////////////////////////////////////
// 2021.02.14 caolei
////////////////////////////////////
#pragma once

#include "catVulkanRender/simplevulkan.h"
#include "catVulkanRender/pipelineKey.h"
#include "catVulkanRender/DescriptorDataKey.h"
#include "catVulkanRender/descriptorAllocator.h"
#include "catVulkanRender/deviceInfo.h"

#include "cat/IRender.h"

#include "cat/color.h"

#include "scl/matrix.h"
#include "scl/vector.h"
#include "scl/hash_table.h"
#include "scl/function.h"

#include "vulkan/vulkan.h"
#include "shaderc/shaderc.h"

// TODO 很多接口  not implemented

struct ImDrawData;

namespace cat {

class DescriptorAllocator;
class CommandAllocator;

class RenderTarget
{
public:
	svkImage		colorImage;
	svkImage		depthImage;
	VkFramebuffer	framebuffer;
};


class VulkanRender : public cat::IRender
{
public:
	VulkanRender();
	~VulkanRender();

	typedef scl::caller_function<void, int, int> FuncOnSurfaceSizeChanged;

	bool					init					(void* hInstance, void* hwnd);
	bool					is_init					()const { return m_isInit; };
	void					swap					();
	void					clear					();
	//void					setOnSurfaceSizeChanged	(void* caller, FuncOnSurfaceSizeChanged::FuncT func) { m_onSurfaceSizeChanged.set(caller, func); }
	void					setOnSurfaceSizeChanged	(FuncOnSurfaceSizeChanged func) { m_onSurfaceSizeChanged = func; }

	void					updateMVP				(const scl::matrix& mvp);
//	void					onResize				(const int width, const int height, bool forceSet = false);
//	void					calcViewMatrix			();
	void					release					();
//	void					scale					(const float v);

//const scl::matrix&		viewMatrix				() const { return m_view; }
//	const scl::matrix&		getMVP					();
//	scl::vector2			unprojectClickPosition	(const float x, const float y);

	//void					prepare					(); // call prepare when render target size changed.
	//void					unprepare				(); 
	void					waitIdle				();

	//implenment IRender

	//vertex
	virtual void*			createVertexBuffer	(const int size);
	virtual void			releaseVertexBuffer	(void* vertexBuffer);
	virtual void			copyVertexBuffer	(const void* data, void* vertexBuffer, const int sizeInByte);//注意，UI使用的vertex的fvf格式是(FVF_XYZ | FVF_DIFFUSE | FVF_TEX1);


	//index
	virtual void*			createIndexBuffer	(const int size);
	virtual void			releaseIndexBuffer	(void* indexBuffer);
	virtual void			copyIndexBuffer		(const void* data, void* indexBuffer, const int sizeInByte);

//	//buffer
//	virtual void*			createBuffer		();
//	virtual void			releaseBuffer		(void* buffer);

	//texture
	virtual void*			createTexture		(const char* const filename, int* width, int* height, int* pitch, PIXEL* pixel);
	virtual void*			createTexture		(const int width, const int height, const PIXEL pixel);
	virtual void			releaseTexture		(void* texture);
	virtual void			copyTexture			(void* texture, const int offset_x, const int offset_y, const int width, const int height, const void* data, const PIXEL pixel, const int alignment);
	void					saveTexture			(void* texture, const char* const filename);
	virtual unsigned char*	loadImage			(const char* const filename, int* width, int* height, int* pitch, PIXEL* pixel);

	//shader
	virtual void*			createShader		(int shaderType);
	virtual void*			createShader		(const char* const vs_code, const char* const ps_code);
	//virtual void			useShader			(void* shader);
	virtual void			releaseShader		(void* shader);

	virtual	void			bindCommandBuffer	();
	virtual	void			unbindCommandBuffer	();

	virtual void			draw2(
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
	);
	
	virtual void			beginDraw			();
	virtual void			endDraw				();
	
	void					beginPickPass		(scl::vector4& clearColorRGBA);
	void					endPickPass			();
	void					savePickPass		();

	void					beginScenePass		(scl::vector4& clearColorRGBA);
	void					endScenePass		();

	//device info
	virtual int				getDeviceWidth		();
	virtual int				getDeviceHeight		();
	const DeviceInfo&		getDeviceInfo		() const { return m_deviceInfo; }

	void					recreateSwapchain	();
	void					recreateSurface		();

	void					initIMGUI			();
	void					releaseIMGUI		();
	void					drawIMGUI			(ImDrawData* draw_data);

	bool					_minimized();

private:
	int						_alignUniformBufferOffset(int size);

	int						_fillUniformData(
		const scl::matrix&		mvp,
		void*					texture,
		const int				jointMatrixCount,
		svkDescriptorData*		uniformDatas,
		const int				uniformDataCapacity);

	void					_prepareDescriptorSet(
		void*					shader, 
		svkDescriptorData*		uniformDatas,
		const int				uniformDataCount,
		DescriptorSet&			descriptorSet,
		DescriptorAllocator*&	descriptorAllocator,
		uint32_t*				dynamicOffsets,
		uint32_t				dynamicOffsetCount);

	void					_preparePipeline(
		const int				primitiveType,
		const VertexAttr*		attrs,
		const int				attrCount,
		void**					vertexBuffers,
		DescriptorAllocator*	descriptorAllocator,
		void*					shader,
		VkRenderPass			renderPass,
		svkPipeline*&			pipeline);

	uint32_t				_fillDynamicOffsets(
		uint32_t*				dynamicOffsets, 
		const int				dynamicOffsetCapacity, 
		const scl::matrix&		mvp,
		const scl::matrix*		jointMatrices,
		const int				jointMatrixCount);

	static RenderTarget		_createRenderTarget		(svkDevice& device, VkFormat colorFormat, VkFormat depthFormat, VkRenderPass renderPass, const int width, const int height);
	static void				_destroyRenderTarget	(svkDevice& device, RenderTarget& renderTarget);
	
	static void				_fillPushConst			(VkCommandBuffer commandBuffer, svkPipeline& pipeline, void* _shader, void* pushConstBuffer, const int pushConstBufferSize);

	//void					_destroyPickRenderTarget	();
	void					_createMainRenderTarget		();
	void					_destroyMainRenderTarget	();

	static void				_fillDeviceInfo				(svkDevice& device, DeviceInfo& info);

private:
	class DrawContext
	{
	public:
		VkRenderPass		renderPass;
		VkFramebuffer		framebuffer;
		int					width;
		int					height;
		svkBuffer			uniform;
		void*				uniformBufferMapped;
		CommandAllocator*	commandAllocator;
		scl::vector4		clearColorRGBA;
	};

private:
	static const int	MAX_FRAME						= svkSwapchain::MAX_IMAGE_COUNT;	// 最大交换帧数量。TODO, 改为动态扩张。开始时2，不够的时候增加
	static const int	MAX_MATRIX_PER_FRAME			= 64;	// assume 64 matrices per object. sizeof(matrix) = 64, but on some old device [limits.minUniformBufferOffsetAlignment] = 256, so use 128 for each matrix.
	static const int	MAX_OBJECT_PER_FRAME			= 1024; // 1024 objects per frame. 
	static const int	MAX_CONFLICT					= 16;	// render 中使用的多个 hash_table 的最大冲突次数。
	static const int	MAX_DESCRITOR_ALLOCATOR_COUNT	= 1024;	// hash_table 中 descriptor allocator 的最大数量
	static const int	MAX_PIPELINE_COUNT				= 1024;	// hash_table 中 pipeline 的最大数量
	static const int	MAX_DESCRIPTOR_SET_CACHE_SIZE	= 1024;	// hash_table 中 descriptor set 的最大数量

	VkInstance			m_inst;
	svkDevice			m_device;
	svkSurface			m_surface;
	svkSwapchain		m_swapchain;
	VkRenderPass		m_mainRenderPass;
	svkImage			m_mainDepthImage;
	svkFrame			m_frames[svkSwapchain::MAX_IMAGE_COUNT];
	int					m_frameCount;
	DeviceInfo			m_deviceInfo;

	// for 3D picking
	VkRenderPass		m_pickRenderPass;
	RenderTarget		m_pickRenderTarget;
	VkCommandBuffer		m_pickCommandBuffer;
	CommandAllocator*	m_pickCommandAllocator;
	VkFence				m_pickFence;
	VkSemaphore			m_pickSemaphore;
	VkCommandBuffer		m_pickCopyCommandBuffer;
	svkBuffer			m_pickPassImageCPUBuffer;
	scl::vector2i		m_pickImageOffset;
	scl::vector2i		m_pickImageSize;

	// for draw context
	DrawContext			m_drawContext;

	bool				m_isInit;
	bool				m_minimized;
	int					m_frameIndex;
	int					m_prevFrameIndex;
	bool				m_matrixChanged;
	float				m_scale;
	svkBuffer			m_frameUniforms[MAX_FRAME];
	void*				m_frameUniformBuffersMapped[MAX_FRAME];
	uint32_t			m_frameUniformBufferOffset;
	CommandAllocator*	m_commandAllocator[MAX_FRAME];
	VkCommandBuffer		m_bindCommandBuffer;


	//for recreate surface
	void*				m_windowInstance;
	void*				m_windowHandle;

	VkDescriptorPool	m_IMGUIDescriptorPool;

	//float				m_clearColor[4];

	scl::hash_table<PipelineKey, svkPipeline*>			m_pipelines;
	scl::hash_table<int, DescriptorAllocator*>			m_descriptorAllocators;		// key 是 uniform bind 的 hash 值
	scl::hash_table<DescriptorDataKey, DescriptorSet>	m_descriptorSetCache;		// 根据不同的 uniform data 来查找对应的 descriptor set
	FuncOnSurfaceSizeChanged							m_onSurfaceSizeChanged;
}; // class VulkanRender

} //namespace cat


