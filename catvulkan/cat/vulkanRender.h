////////////////////////////////////
// 2021.02.14 caolei
////////////////////////////////////
#pragma once

#include "cat/simplevulkan.h"
#include "cat/pipelineKey.h"
#include "cat/DescriptorDataKey.h"
#include "cat/descriptorAllocator.h"
#include "cat/deviceInfo.h"

#include "cat/IRender.h"
#include "cat/color.h"

#include "scl/matrix.h"
#include "scl/vector.h"
#include "scl/hash_table.h"
#include "scl/function.h"

#include "vulkan/vulkan.h"
#include "shaderc/shaderc.h"

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


// VulkanRender 必须由单一线程使用（典型情况是主线程）。
// presentCallback 由 svkPresent 在调用线程同步触发，所以 _prepareDescriptorSet / _preparePipeline 等
// 内部访问的 hash_table 都不加锁。如果未来要支持多线程渲染，需要重新评估锁策略。
class VulkanRender : public cat::IRender
{
public:
	VulkanRender();
	~VulkanRender();

	bool					init					(void* hInstance, void* hwnd);
	bool					is_init					()const { return m_isInit; };
	void					swap					();
	void					clear					();
	void					setOnSurfaceResize		(scl::class_function<void (int, int)> func) { m_onSurfaceResize = func; }
	void					setReverseZ				(bool v) { m_reverseZ = v; }

	void					updateMVP				(const scl::matrix& mvp);
//	void					onResize				(const int width, const int height, bool forceSet = false);
//	void					calcViewMatrix			();
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
	virtual void			writeVertexBuffer	(const void* src, void* dstVertexBuffer, const int sizeInByte);
	virtual void			readVertexBuffer	(void* dst, void* srcVertexBuffer, const int sizeInByte, const int offset = 0);
	//virtual void*			mapVertexBuffer		(void* vertexBuffer);
	//virtual void			unmapVertexBuffer	(void* vertexBuffer);

	//index
	virtual void*			createIndexBuffer	(const int size);
	virtual void			releaseIndexBuffer	(void* indexBuffer);
	virtual void			writeIndexBuffer	(const void* src, void* dstIndexBuffer, const int sizeInByte);
	virtual void			readIndexBuffer		(void* dst, void* srcIndexBuffer, const int sizeInByte, const int offset = 0);
	//virtual void*			mapIndexBuffer		(void* indexBuffer);
	//virtual void			unmapIndexBuffer	(void* indexBuffer);

	//texture
	virtual void*			createTexture		(const char* const filename, int* width, int* height, int* pitch, PIXEL* pixel);
	virtual void*			createTexture		(const int width, const int height, const PIXEL pixel);
	virtual void			releaseTexture		(void* texture);
	virtual void			copyTexture			(void* texture, const int offset_x, const int offset_y, const int width, const int height, const void* data, const PIXEL pixel, const int alignment);
	void					saveTexture			(void* texture, const char* const filename);
	virtual unsigned char*	loadImage			(const char* const filename, int* width, int* height, int* pitch, PIXEL* pixel);

	//shader
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
	scl::vector4			endPickPass			(int x, int y);
	scl::vector4			savePickPass		(int width, int height);

	void					beginScenePass		(scl::vector4& clearColorRGBA);
	void					endScenePass		();

	//device info
	virtual int				getDeviceWidth		() const;
	virtual int				getDeviceHeight		() const;
	const DeviceInfo&		getDeviceInfo		() const { return m_deviceInfo; }
	void*					getWindowHandle		() const { return m_windowHandle; }
	void*					getWindowInstance	() const { return m_windowInstance; }

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

	DescriptorSet			_prepareDescriptorSet(
		void*					shader, 
		svkDescriptorData*		descriptorDatas,
		const int				descriptorDataCount,
		VkDescriptorSetLayout&	descriptorSetLayout,
		uint32_t*				dynamicOffsets,
		uint32					dynamicOffsetCount);

	void					_preparePipeline(
		const int				primitiveType,
		const VertexAttr*		attrs,
		const int				attrCount,
		void**					vertexBuffers,
		VkDescriptorSetLayout	descriptorSetLayout,
		void*					shader,
		VkRenderPass			renderPass,
		svkPipeline*&			pipeline);

	uint32_t				_fillDynamicOffsets(
		uint32_t*				dynamicOffsets, 
		const int				dynamicOffsetCapacity, 
		const scl::matrix&		mvp,
		const scl::matrix*		jointMatrices,
		const int				jointMatrixCount);

	bool					_prepareDescriptorSetAndFillData(
		void*					shader, 
		const scl::matrix&		mvp,
		void*					texture,
		const scl::matrix*		jointMatrices,
		const int				jointMatrixCount,
		DescriptorSet&			outputDescriptorSet,
		VkDescriptorSetLayout&	outputDescriptorSetLayout,
		uint32_t*				outputDynamicOffsets,
		uint32_t				outputDynamicOffsetCapacity,
		uint32_t&				outputDynamicOffsetCount);

	static RenderTarget		_createRenderTarget		(svkDevice& device, VkFormat colorFormat, VkFormat depthFormat, VkRenderPass renderPass, const int width, const int height);
	static void				_destroyRenderTarget	(svkDevice& device, RenderTarget& renderTarget);
	
	static void				_fillPushConst			(VkCommandBuffer commandBuffer, svkPipeline& pipeline, void* _shader, void* pushConstBuffer, const int pushConstBufferSize);

	//void					_destroyPickRenderTarget	();
	void					_createMainRenderTarget		();
	void					_destroyMainRenderTarget	();

	static void				_fillDeviceInfo				(svkDevice& device, DeviceInfo& info);

private:
	// 注意：DrawContext 必须保持 POD（trivially copyable）。
	// VulkanRender ctor 用 memclr(m_drawContext) 整片清零，endScenePass / endPickPass 末尾也用 memclr 清场。
	// 加非 POD 字段（智能指针、std::function、含 vtable 的类等）会让 memclr 变成 UB。
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
	static const int	MAX_JOINT_PER_OBJECT			= 64;	// 每个蒙皮 object 的最大骨骼数；骨骼矩阵被打包成一段连续 buffer 写入
	static const int	MAX_DRAW_PER_FRAME				= 1024;	// 每帧最大 draw 调用数（_fillDynamicOffsets 入口 assert，release 下跳过 draw）
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
	VkFormat			m_colorFormat;
	VkFormat			m_depthFormat;

	// for 3D picking
	VkRenderPass		m_pickRenderPass;
	RenderTarget		m_pickRenderTarget;
	VkCommandBuffer		m_pickCommandBuffer;
	CommandAllocator*	m_pickCommandAllocator;
	VkFence				m_pickFence;
	VkSemaphore			m_pickSemaphore;
	VkCommandBuffer		m_pickCopyCommandBuffer;
	svkBuffer			m_pickPassImageCPUBuffer;

	// for draw context
	DrawContext			m_drawContext;

	bool				m_isInit;
	bool				m_minimized;
	bool				m_frameAcquired;					// 本帧是否已成功 acquire swapchain image
	bool				m_frameSubmitted;					// 本帧是否已 submit main scene，swap 只能 present 已 submit 的 image
	int					m_frameIndex;
	int					m_prevFrameIndex;
	bool				m_matrixChanged;
	float				m_scale;
	svkBuffer			m_frameUniforms[MAX_FRAME];
	void*				m_frameUniformBuffersMapped[MAX_FRAME];
	uint32_t			m_frameUniformBufferOffset;
	uint32_t			m_frameUniformBudget;			// 单帧 uniform buffer 字节预算 = (mvp 步长 + 骨骼段步长) × MAX_DRAW_PER_FRAME，init 时算一次
	uint32_t			m_frameDrawCount;				// 当前帧已发出的 draw 数（_fillDynamicOffsets 每次 +1，beginDraw 复位），与 MAX_DRAW_PER_FRAME 对比
	CommandAllocator*	m_commandAllocator[MAX_FRAME];
	VkCommandBuffer		m_bindCommandBuffer;
	bool				m_reverseZ;


	//for recreate surface
	void*				m_windowInstance;
	void*				m_windowHandle;

	VkDescriptorPool	m_IMGUIDescriptorPool;

	scl::hash_table<PipelineKey, svkPipeline*>			m_pipelines;
	scl::hash_table<int, DescriptorAllocator*>			m_descriptorAllocators;		// key 是 uniform bind 的 hash 值
	scl::hash_table<DescriptorDataKey, DescriptorSet>	m_descriptorSetCache;		// 根据不同的 uniform data 来查找对应的 descriptor set
	scl::class_function<void (int, int)>				m_onSurfaceResize;
}; // class VulkanRender

} //namespace cat


