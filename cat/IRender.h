#pragma once

#include "gfx/color.h"

#include "cat/def.h"

namespace scl { class matrix; }

namespace cat {

// see the shader example at the bottom of this file.
enum SHADER
{
	SHADER_INVALID = -1,

	SHADER_DEFAULT,
	SHADER_ADD_COLOR,
	SHADER_COLOR,
	SHADER_FONT,
	SHADER_FONT_OUTLINE,
	SHADER_GRAY,
	SHADER_VIDEO,

	SHADER_COUNT,
};

enum CAMERA
{
	CAMERA_ORTHO,		//user defined camera with ortho projection. 
	CAMERA_PERSPECTIVE,	//user defined camera with perspective projection.
	CAMERA_WORLD,		//camera is auto selected by game world.
};


// Same as opengl's glDrawPrimitive(mode) and cgltf_primitive_type
// !!! Dont' change this !!!
enum PRIMITIVE_TYPE
{
	PRIMITIVE_TYPE_POINTS,
	PRIMITIVE_TYPE_LINES,
	PRIMITIVE_TYPE_LINE_LOOP,
	PRIMITIVE_TYPE_LINE_STRIP,
	PRIMITIVE_TYPE_TRIANGLES,
	PRIMITIVE_TYPE_TRIANGLE_STRIP,
	PRIMITIVE_TYPE_TRIANGLE_FAN,
};

class VertexAttr
{
public:
	int					index;
	int					size;
	ELEM_TYPE	dataType;
	int					normalize;
	int					stride;
	void*				offset;
};

//class UniformBind
//{
//public:
//	enum TYPE
//	{
//		TYPE_BUFFER,
//		TYPE_BUFFER_DYNAMIC,
//		TYPE_SAMPLER,
//	};
//	enum STAGE
//	{
//		STAGE_VERT,
//		STAGE_FRAG,
//	};
//	int		bind;
//	TYPE	type;
//	STAGE	stage;
//	int		count;
//};

//class UniformData
//{
//public:
//	//enum TYPE
//	//{
//	//	TYPE_BUFFER,
//	//	TYPE_BUFFER_DYNAMIC,
//	//	TYPE_SAMPLER,
//	//};
//	//enum STAGE
//	//{
//	//	STAGE_VERT,
//	//	STAGE_FRAG,
//	//};
//
//	struct BufferInfo
//	{
//		const void* buffer;
//		int bufferSize;
//	};
//	struct TextureInfo
//	{
//		void* texture;
//	};
//	union Info 
//	{
//		BufferInfo	buffer;
//		TextureInfo texture;
//	};
//
//	Info data[64];
//	int dataCount;
//};

class IRender
{
public:
	virtual ~IRender() {} 

	//vertex
	virtual void*			createVertexBuffer	(const int vertexCount) = 0;
	virtual void			releaseVertexBuffer	(void* vertexBuffer) = 0;
	virtual void			copyVertexBuffer	(const void* data, void* vertexBuffer, const int sizeInByte) = 0;//注意，UI使用的vertex的fvf格式是(FVF_XYZ | FVF_DIFFUSE | FVF_TEX1);

	//index
	virtual void*			createIndexBuffer	(const int vertexCount) = 0;
	virtual void			releaseIndexBuffer	(void* vertexBuffer) = 0;
	virtual void			copyIndexBuffer		(const void* data, void* vertexBuffer, const int sizeInByte) = 0;

	//virtual void*			createBuffer		() = 0;
	//virtual void			releaseBuffer		(void* vertexBuffer) = 0;

	//texture
	virtual void*			createTexture		(const char* const filename, int* width, int* height, int* pitch, gfx::PIXEL* pixel) = 0;
	virtual void*			createTexture		(const int width, const int height, const gfx::PIXEL pixel) = 0;
	virtual void			releaseTexture		(void* texture) = 0;
	virtual void			copyTexture			(void* texture, const int offset_x, const int offset_y, const int width, const int height, const void* data, const gfx::PIXEL pixel, const int alignment) = 0;
	virtual void			saveTexture			(void* texture, const char* const filename) = 0;
	virtual unsigned char*	loadImage			(const char* const filename, int* width, int* height, int* pitch, gfx::PIXEL* pixel) = 0;

	//shader
	virtual void*			createShader		(int shader) = 0;
	virtual void*			createShader		(const char* const vs_code, const char* const ps_code) = 0;
	virtual void			releaseShader		(void* shader) = 0;

	//uniform
	//virtual void*			createUniformLayout	(const UniformBind* binds, const int bindCount) = 0;

	//draw
	//virtual void			draw				(
	//	void*		geometry, 
	//	void*		texture, 
	//	void*		vertexBuffer, 
	//	void*		indexBuffer, 
	//	const int	primitiveCount, 
	//	int			attrCount,
	//	VertexAttr* attrs,
	//	const gfx::ALPHA_MODE alphaMode, 
	//	scl::matrix* transform, 
	//	void* shader, 
	//	const char* const uniformName, 
	//	const uint uniformValue, 
	//	void* camera, 
	//	const int//renderLevel
	//	) = 0;

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
		) = 0;

	//debug for vulkan
	virtual void beginDraw() = 0;
	virtual void endDraw() = 0;

	//device info
	virtual int				getDeviceWidth		()	= 0;
	virtual int				getDeviceHeight		()	= 0;

	//camera
	//virtual void*			createCamera		(CAMERA type, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ, float posX, float posY, float posZ, float fovy, float aspect, float nearZ, float farZ)	= 0;
	//virtual void			releaseCamera		(void* camera)				= 0;
	//virtual void			get2DPosition		(void* camera, const float screenX, const float screenY, const float width, const float height, const float posX, const float posY, float& outX, float& outY) = 0;

	//model
	//virtual void*			createModel			(const char* const modelName, const char* const params) = 0;
	//virtual float			modifyModelParam	(void* model, const char* const params) = 0;
	//virtual void*			renderModel			(void* model, const float x, const float y, const float width, const float height, float& texLeft, float& texTop, float& texRight, float& texBottom) = 0;  // return a device/render texture
	//virtual void			releaseModel		(void* model) = 0;

	//effect
	//virtual void*			createEffect		(const char* const effectName, const char* const params) = 0;
	// modifyEffectParam examples: 
	//		modifyEffectParam(effect, "start")		// start play effect
	//		modifyEffectParam(effect, "stop")		// stop playing effect
	//		modifyEffectParam(effect, "length")		// get the effect play length
	//		modifyEffectParam(effect, "scaleX:2")	// scale the effect width by 2 times.
	//		modifyEffectParam(effect, "scaleY:2")	// scale the effect height by 2 times.
	//		modifyEffectParam(effect, "isPlaying")	// if the effect is playing.
	//		modifyEffectParam(effect, "alpha:0.5"))	// set the effect alpha to 0.5 (0.0 ~ 1.0)
	//		modifyEffectParam(effect, "rotate:30"))	// rotate the effect 30 angle around Z axis
	//virtual float			modifyEffectParam	(void* model, const char* const params) = 0;
	//virtual void			renderEffect		(const int effectCount, void** effects, const float* x, const float* y, const float* width, const float* height, const bool* needScissor, const float* scissorX, const float* scissorY, const float* scissorWidth, const float* scissorHeight, const float* alpha, const int renderLevel) = 0;
	//virtual void			releaseEffect		(void* effect) = 0;

	//sound
	//virtual void			playSound			(const char* const name) = 0;

	//geometry (if some engine's renderer need)
	//virtual void			releaseGeometry		(void* geometry) = 0;

	//video
	//virtual void*			createVideo			(const char* const videoName, void** videoDeviceTexture, float* texcoord_left, float* texcoord_top, float* texcoord_right, float* texdoord_bottom) = 0;
	//		modifyVideo(video, "play")		// start to play the video
	//		modifyVideo(video, "stop")		// stop playing video
	//		modifyVideo(video, "restart")	// restart the video
	//		modifyVideo(video, "pause")		// pause playing video
	//		modifyVideo(video, "isPlaying") // return whether the video is playing
	//virtual int				modifyVideo			(void* video, const char* const params) = 0;
	//virtual void			releaseVideo		(void* video) = 0;
};

} //namespace ui


//	Shader examples
//
//
//	//------SHADER_UI pixel shader------//
//
//	static const char* const SHADER_UI_ps = 
//	"#version 300 es											\n"
//	"precision mediump float;									\n"
//	"in vec4 vertex_color;										\n"
//	"in vec2 vertex_coord;										\n"
//	"out vec4 out_color;										\n"
//	"uniform sampler2D tex;										\n"
//	"void main()												\n"
//	"{															\n"
//	"	out_color = vertex_color * texture(tex, vertex_coord);	\n"
//	"}															\n";
//
//
//
//	//------SHADER_UI_ADD_COLOR pixel shader------//
//
//	static const char* const SHADER_UI_ADD_COLOR_ps = 
//	"#version 300 es											\n"
//	"precision mediump float;									\n"
//	"in vec4 vertex_color;										\n"
//	"in vec2 vertex_coord;										\n"
//	"out vec4 out_color;										\n"
//	"uniform sampler2D tex;										\n"
//	"void main()												\n"
//	"{															\n"
//	"	vec4 tex_color	= texture(tex, vertex_coord);			\n"
//	"	out_color.a		= vertex_color.a * tex_color.a;			\n"
//	"	out_color.rgb	= vertex_color.rgb + tex_color.rgb;		\n"
//	"}															\n";
//
//
//
//
//	//------SHADER_COLOR pixel shader------//
//
//	static const char* const SHADER_COLOR_ps = 
//	"#version 300 es											\n"
//	"precision mediump float;									\n"
//	"in vec4 vertex_color;										\n"
//	"in vec2 vertex_coord;										\n"
//	"out vec4 out_color;										\n"
//	"uniform sampler2D tex;										\n"
//	"void main()												\n"
//	"{															\n"
//	"	out_color = vertex_color;								\n"
//	"}															\n";
//
//
//
//
//	//------SHADER_FONT pixel shader------//
//
//	static const char* const SHADER_FONT_ps =
//	"#version 300 es										\n"
//	"precision mediump float;								\n"
//	"in vec4 vertex_color;									\n"
//	"in vec2 vertex_coord;									\n"
//	"out vec4 out_color;									\n"
//	"uniform sampler2D tex;									\n"
//	"void main()											\n"
//	"{														\n"
//	"	vec4 tex_color = texture(tex, vertex_coord);		\n"
//	"	out_color = vec4(vertex_color.rgb , tex_color.a);	\n"
//	"}														\n";
//
//
//
// 
//	//------SHADER_FONT_OUTLINE pixel shader------//
//
//	static const char* const SHADER_FONT_OUTLINE_ps =
//	"#version 300 es																\n"
//	"precision mediump float;														\n"
//	"in vec4 vertex_color;															\n"
//	"in vec2 vertex_coord;															\n"
//	"out vec4 out_color;															\n"
//	"uniform sampler2D tex;															\n"
//	"uniform vec4 outline_color;													\n"
//	"void main()																	\n"
//	"{																				\n"
//	"	vec4 tex_color		= texture(tex, vertex_coord);							\n"
//	"	float fontAlpha		= tex_color.a; 											\n"
//	"	float outlineAlpha	= tex_color.r; 											\n"
//	"	vec4 color = vertex_color * fontAlpha + outline_color * (1.0 - fontAlpha);	\n"
//	"	out_color = vec4(color.rgb , max(fontAlpha, outlineAlpha) * color.a);		\n"
//	"}																				\n";
//
//
//	//------SHADER_GRAY pixel shader------//
//
//	static const char* const SHADER_GRAY_ps = 
//	"#version 300 es											\n"
//	"precision mediump float;									\n"
//	"in vec4 vertex_color;										\n"
//	"in vec2 vertex_coord;										\n"
//	"out vec4 out_color;										\n"
//	"uniform sampler2D tex;										\n"
//	"void main()												\n"
//	"{															\n"
//	"	out_color = vertex_color * texture(tex, vertex_coord);		\n"
//	"	float gray = dot(out_color.rgb, vec3(0.299, 0.587, 0.114));	\n"
//	"	out_color.rgb = vec3(gray, gray, gray);						\n"
//	"}		
//
//	//------VERTEX SHADER------//
//	//------all shader use same vertext shader------//
//
//	static const char* const vs = 
//	"#version 300 es											\n"
//	"uniform mat4 mvp;											\n"
//	"layout(location = 0) in vec4 position;						\n"
//	"layout(location = 1) in vec4 color;						\n"
//	"layout(location = 2) in vec2 coord;						\n"
//	"out vec4 vertex_color;										\n"
//	"out vec2 vertex_coord;										\n"
//	"void main()												\n"
//	"{															\n"
//	"   gl_Position 			= mvp * position;				\n"
//	"   vertex_color.rgba 		= color.bgra;					\n"
//	"   vertex_coord			= coord;						\n"
//	"}															\n";
//



