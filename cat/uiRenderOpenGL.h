#pragma once

#include "cat/IRender.h"

#include "cat/color.h"

#ifdef __APPLE__
#else
#include "cat/eglWindow.h"
#endif

#include "scl/matrix.h"
#include "scl/vector.h"

namespace cat {

class Device;

class UIRenderOpenGL : public cat::IRender
{
public:
	UIRenderOpenGL();
	~UIRenderOpenGL();

	//device
#ifdef __APPLE__
	bool					init					(const int width, const int height);
#else
	bool					init					(void* hInstance, void* hwnd);
#endif
	bool					is_init					()const { return m_init; };
	void					swap					();
	void					clear					();
	void					onResize				(const int width, const int height, bool forceSet = false);
	//void					calcViewMatrix			();
	void					release					();
	void					scale					(const float v);

	//const scl::matrix&		viewMatrix				() const { return m_view; }
	//const scl::matrix&		getMVP					();
	//scl::vector2			unprojectClickPosition	(const float x, const float y);

	//implenment IRender

	//vertex
	virtual void*			createVertexBuffer	(const int size);
	virtual void			releaseVertexBuffer	(void* vertexBuffer);
	virtual void			copyVertexBuffer	(const void* data, void* vertexBuffer, const int sizeInByte);//注意，UI使用的vertex的fvf格式是(FVF_XYZ | FVF_DIFFUSE | FVF_TEX1);

	//index
	virtual void*			createIndexBuffer	(const int size);
	virtual void			releaseIndexBuffer	(void* indexBuffer);
	virtual void			copyIndexBuffer		(const void* data, void* indexBuffer, const int sizeInByte);

	//buffer
	//virtual void*			createBuffer		();
	//virtual void			releaseBuffer		(void* buffer);

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
	virtual void			useShader			(void* shader);
	virtual void			releaseShader		(void* shader);
	//uniform
	//virtual void*			createUniformLayout	(const UniformBind* binds, const int bindCount) { return NULL; }

	//draw
	//virtual void			draw(
	//	void*		geometry, 
	//	void*		texture, 
	//	void*		vertexBuffer, 
	//	void*		indexBuffer, 
	//	const int	primitiveCount, 
	//	int			attrCount,
	//	VertexAttr* attrs,
	//	//int attrIndexArray,
	//	//int attrSizeArray,
	//	//int attrOffsetArray,
	//	//int attrTypeArray,
	//	//int attrNormalizeArray,
	//	const gfx::ALPHA_MODE alphaMode, 
	//	scl::matrix* transform, 
	//	void* shader, 
	//	const char* const uniformName, 
	//	const uint uniformValue, 
	//	void* camera, 
	//	const int//renderLevel
	//	);

	virtual void draw2(
		void*		texture, 
		void**		vertexBuffers, 
		const int	primitiveType,
		void*		indexBuffer, 
		const int	indexCount, 
		const int	indexComponentType,
		const int	indexOffset,
		int			attrCount,
		const VertexAttr* attrs,
		void*		shader,
		const scl::matrix& mvp,
		const scl::matrix* jointMatrices,
		const int jointMatrixCount
	);
	
	virtual void beginDraw() {};
	virtual void endDraw() {};

	//device info
	virtual int				getDeviceWidth		();
	virtual int				getDeviceHeight		();

	//virtual void*			createCamera		(ui::CAMERA type, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ, float posX, float posY, float posZ, float fovy, float aspect, float nearZ, float farZ);
	//virtual void			releaseCamera		(void* camera);
	//virtual void			get2DPosition		(void* camera, const float screenX, const float screenY, const float width, const float height, const float posX, const float posY, float& outX, float& outY);

	//virtual void*			createModel			(const char* const modelName, const char* const params);
	//virtual void*			renderModel			(void* model, const float x, const float y, const float width, const float height, float& texLeft, float& texTop, float& texRight, float& texBottom);
	//virtual void			releaseModel		(void* model);
	//virtual float			modifyModelParam	(void* , const char* const ) { return 0; }


	//virtual void*			createEffect		(const char* const , const char* const );
	//virtual float			modifyEffectParam	(void* , const char* const ) { return 0; }
	//virtual void			renderEffect(const int effectCount, void** effects, const float* x, const float* y, const float* width, const float* height, const bool* needScissor, const float* scissorX, const float* scissorY, const float* scissorWidth, const float* scissorHeight, const float* alpha, const int renderLevel);
	//virtual void			releaseEffect		(void* ) {}
	
	//sound
	//virtual void			playSound			(const char* const) {}

	//geometry (if some engine's renderer need)
	//virtual void			releaseGeometry		(void* ) {}

	//video
	//virtual void*			createVideo			(const char* const videoName, void** videoDeviceTexture, float* texcoord_left, float* texcoord_top, float* texcoord_right, float* texdoord_bottom);
	//		modifyVideo(video, "play")		// start to play the video
	//		modifyVideo(video, "stop")		// stop playing video
	//		modifyVideo(video, "restart")	// restart the video
	//		modifyVideo(video, "pause")		// pause playing video
	//		modifyVideo(video, "isPlaying") // return whether the video is playing
	//virtual int				modifyVideo			(void* video, const char* const params);
	//virtual void			releaseVideo		(void* video);

private:
	//void					_update_mvp			(const uint shader);
	void					_use_program		(const uint shader);
	void					_set_texture_param	();
	//void					_set_world_transform(void* shader, scl::matrix& m);
	void					_set_uniform		(void* shader, const char* const name, const uint color);

private:
	static const int	VBO_SIZE = 5;

#ifdef __APPLE__
	int					m_width;
	int					m_height;
#else
	EGLWindow			m_eglWindow;
#endif
	bool				m_init;
	int					m_uniform_tex;
	int					m_uniform_mvp;
	int					m_uniform_jointMatrix;
	//scl::matrix			m_view;
	//scl::matrix			m_world;
	int					m_alignment;
	bool				m_matrix_changed;
	float				m_scale;
	uint				m_effectShader;	
};

} //namespace cat

