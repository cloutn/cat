#include "./uiRenderOpenGL.h"

#include "libimg/image.h"
#include "cat/shader_gles.h"
#include "cat/vertex.h"

#ifdef __APPLE__
#include <OpenGLES/ES3/gl.h>
#else
#include "GLES3/gl3.h"
#endif

#include "scl/type.h"
#include "scl/assert.h"
#include "scl/stringdef.h"
#include "scl/matrix.h"
#include "scl/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <memory.h>



namespace cat {

using cat::vertex_color_uv;

UIRenderOpenGL::UIRenderOpenGL() : 
#ifdef __APPLE__
	m_width				(0),
	m_height			(0),
#endif
	//m_shader			(0), 
	m_init				(false), 
	m_uniform_tex		(-1), 
	m_uniform_mvp		(-1),
	m_uniform_jointMatrix(-1),
	m_alignment			(4),
	m_matrix_changed	(false),
	m_scale				(1.0f),
	m_effectShader		(0)
{
	//m_view = scl::matrix::identity();
	//m_world = scl::matrix::identity();
}


void* UIRenderOpenGL::createVertexBuffer(const int vertexCount)
{
	uint buf = 0;
	glcheck( glGenBuffers(1, &buf) );
	return reinterpret_cast<void*>(static_cast<uintptr_t>(buf));
}

void UIRenderOpenGL::releaseVertexBuffer(void* vertexBuffer)
{
	if (NULL == vertexBuffer)
		return;

	glcheck( glDeleteBuffers(1, reinterpret_cast<uint*>(&vertexBuffer)) );
}

void* UIRenderOpenGL::createIndexBuffer(const int vertexCount)
{
	uint buf = 0;
	glcheck( glGenBuffers(1, &buf) );
	return reinterpret_cast<void*>(static_cast<uintptr_t>(buf));
}

void UIRenderOpenGL::releaseIndexBuffer(void* vertexBuffer)
{
	if (NULL == vertexBuffer)
		return;

	glcheck( glDeleteBuffers(1, reinterpret_cast<uint*>(&vertexBuffer)) );
}

//void* UIRenderOpenGL::createBuffer()
//{
//	uint buf = 0;
//	glcheck( glGenBuffers(1, &buf) );
//	return reinterpret_cast<void*>(buf);
//}
//
//void UIRenderOpenGL::releaseBuffer(void* buffer)
//{
//	if (NULL == buffer)
//		return;
//
//	glcheck( glDeleteBuffers(1, reinterpret_cast<uint*>(&buffer)) );
//}

void* UIRenderOpenGL::createTexture(const char* const filename, int* width, int* height, int* pitch, PIXEL* pixel)
{
	//char s[256] = { 0 };
	//scl::wchar_to_ansi(s, 256, filename, static_cast<int>(wcslen(filename)), scl::Encoding_UTF8);

	//创建纹理   
	uint textureID = img::load_img(filename, width, height, pitch, (int*)pixel);
	
	_set_texture_param();

	glcheck( glBindTexture(GL_TEXTURE_2D, 0) );
	return reinterpret_cast<void*>(static_cast<uintptr_t>(textureID));
}

unsigned char* UIRenderOpenGL::loadImage(const char* const filename, int* width, int* height, int* pitch, PIXEL* pixel)
{
	int _pix = -1;
	unsigned char* data = img::load_png_data(filename, NULL, width, height, pitch, &_pix);
	if (_pix == 4)
		*pixel = cat::PIXEL_RGBA;
	else if (_pix == 3)
		*pixel = cat::PIXEL_RGB;

	return data;
}

GLint pixel_to_format(const PIXEL pixel)
{
	GLint format = GL_RGBA;
	switch (pixel)
	{
	case cat::PIXEL_A		: format	= GL_ALPHA;				break;
	case cat::PIXEL_L		: format	= GL_LUMINANCE;			break;
	case cat::PIXEL_LA		: format	= GL_LUMINANCE_ALPHA;	break;
	case cat::PIXEL_RGB		: format	= GL_RGB;				break;
	case cat::PIXEL_RGBA	: format	= GL_RGBA;				break;
	default: assert(false); break;
	}
	return format;
}


GLint pixel_to_internal_format(const cat::PIXEL pixel)
{
	GLint internalFormat = GL_RGBA8;
	switch (pixel)
	{
	case cat::PIXEL_A		: internalFormat	= GL_ALPHA; break;
	case cat::PIXEL_L		: internalFormat	= GL_LUMINANCE; break;
	case cat::PIXEL_LA		: internalFormat	= GL_LUMINANCE_ALPHA; break;
	case cat::PIXEL_RGB		: internalFormat	= GL_RGB8; break;
	case cat::PIXEL_RGBA	: internalFormat	= GL_RGBA8; break;
	default: assert(false); break;
	}
	return internalFormat;
}


void* UIRenderOpenGL::createTexture(const int width, const int height, const cat::PIXEL pixel)
{
	uint tex = 0;

	//TODO empty太费内存了
	int BYTE_SIZE			= sizeof(uint) * width * height;
	uint* empty				= (uint*)malloc(BYTE_SIZE);
	GLint internalFormat	= pixel_to_internal_format(pixel);
	GLint format			= pixel_to_format(pixel);
	memset((byte*)empty, 0x00,	BYTE_SIZE);

	glcheck( glGenTextures(1, &tex) );
	glcheck( glBindTexture(GL_TEXTURE_2D, tex) );
	glcheck( glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, empty) );
	_set_texture_param();
	glcheck( glBindTexture(GL_TEXTURE_2D, 0) );

	free(empty);
	return reinterpret_cast<void*>(static_cast<uintptr_t>(tex));
}

void UIRenderOpenGL::copyTexture(void* texture, const int offset_x, const int offset_y, const int width, const int height, const void* data, const cat::PIXEL pixel, const int alignment)
{
	uint tex = static_cast<uint>(reinterpret_cast<uint64>(texture));
	glcheck( glBindTexture(GL_TEXTURE_2D, tex) );
	GLint format = pixel_to_format(pixel);
	int old_align = 0;
	glcheck( glGetIntegerv(GL_UNPACK_ALIGNMENT, &old_align) );
	if (old_align != alignment)
	{
		glcheck( glPixelStorei(GL_UNPACK_ALIGNMENT, alignment) );
	}
	//if (alignment != m_alignment)
	//{
	//	glcheck(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
	//	m_alignment = alignment;
	//}
	glcheck(glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y, width, height, format, GL_UNSIGNED_BYTE, data));
	glcheck(glBindTexture(GL_TEXTURE_2D, 0));
	glcheck( glPixelStorei(GL_UNPACK_ALIGNMENT, old_align) );
}

void UIRenderOpenGL::releaseTexture(void* texture)
{
	if (NULL == texture)
		return;
	uint tex = static_cast<uint>(reinterpret_cast<uint64>(texture));
	glcheck( glDeleteTextures(1, &tex) );
}


void UIRenderOpenGL::copyVertexBuffer(const void* data, void* vertexBuffer, const int sizeInByte)
{
	uint vbo = static_cast<uint>(reinterpret_cast<uint64>(vertexBuffer));
	glcheck( glBindBuffer(GL_ARRAY_BUFFER, vbo) );
	glcheck( glBufferData(GL_ARRAY_BUFFER, sizeInByte, data, GL_STATIC_DRAW) );
	glcheck( glBindBuffer(GL_ARRAY_BUFFER, 0) );
}

int VertexAttrDataTypeToGLDataType(ELEM_TYPE t)
{
	switch (t)
	{
	case ELEM_TYPE_INT8				: return GL_BYTE;
	case ELEM_TYPE_UINT8	: return GL_UNSIGNED_BYTE;
	case ELEM_TYPE_INT16			: return GL_SHORT;
	case ELEM_TYPE_UINT16	: return GL_UNSIGNED_SHORT;
	case ELEM_TYPE_INT32				: return GL_INT;
	case ELEM_TYPE_UINT32		: return GL_UNSIGNED_INT;
	case ELEM_TYPE_FLOAT			: return GL_FLOAT;
	//case ELEM_TYPE_DOUBLE			: return GL_DOUBLE;
	default:
		assert(false);
		break;
	};
}

//void UIRenderOpenGL::draw(
//	void* geometry, 
//	void* texture, 
//	void* vertexBuffer, 
//	void* indexBuffer, 
//	const int primitiveCount, 
//	const int attrCount,
//	VertexAttr* attrs,
//	const cat::ALPHA_MODE alphaMode, 
//	scl::matrix* transform, 
//	void* shader, 
//	const char* const uniformName, 
//	const uint uniformValue, 
//	void* camera, 
//	const int//renderLevel
//	)
//{
//	_set_world_transform(shader, *transform);
//
//	if (uniformValue > 0)
//		_set_uniform(shader, uniformName, uniformValue);
//
//	// texture
//	uint tex = static_cast<uint>(reinterpret_cast<uint64>(texture));
//	glcheck( glBindTexture(GL_TEXTURE_2D, tex) );
//
//	//// buffer
//	glcheck( glBindBuffer(GL_ARRAY_BUFFER,			static_cast<uint>(reinterpret_cast<uint64>(vertexBuffer))) );
//	glcheck( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	static_cast<uint>(reinterpret_cast<uint64>(indexBuffer))) );
//
//	//// vertex attribute 
//	for (int i = 0; i < attrCount; ++i)
//	{
//		VertexAttr a = attrs[i];
//		glcheck( glEnableVertexAttribArray(a.index) );
//		glcheck( glVertexAttribPointer(a.index, a.size, VertexAttrDataTypeToGLDataType(a.dataType), a.normalize, a.stride, (void*)a.offset) );
//	}
//	//glcheck( glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_color), 0) );
//	//glcheck( glEnableVertexAttribArray(0) );
//
//	//glcheck( glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex_color), offset(vertex_color, color)) );
//	//glcheck( glEnableVertexAttribArray(1) );
//
//	//glcheck( glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_color), offset(vertex_color, u)) );
//	//glcheck( glEnableVertexAttribArray(2) );
//
//	//glDisable(GL_BLEND);
////	glcheck( glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(vertex_color), offset(vertex_color, index)) );
////	glcheck( glEnableVertexAttribArray(3) );
//
//	//blend
//	//if (alphaMode == cat::ALPHA_MODE_ADD)
//	//	glcheck( glBlendFunc(GL_DST_ALPHA, GL_ONE) );
//
//	glcheck( glDrawElements(GL_TRIANGLES, primitiveCount * 3, GL_UNSIGNED_SHORT, 0) );
//
//	//if (alphaMode == cat::ALPHA_MODE_ADD)
//	//	glcheck( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
//
//	//clear
//	glcheck( glDisableVertexAttribArray(0) );
//	glcheck( glDisableVertexAttribArray(1) );
//	glcheck( glDisableVertexAttribArray(2) );
//	glcheck( glBindVertexArray(0) );
//	glcheck( glBindTexture(GL_TEXTURE_2D, 0) );
//	glcheck( glBindBuffer(GL_ARRAY_BUFFER, 0) );
//	glcheck( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
//}
//


void UIRenderOpenGL::draw2(
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
	)
{
	const uint _shader = static_cast<uint>(reinterpret_cast<uint64>(shader));
	_use_program(_shader);
	m_uniform_mvp = glGetUniformLocation(_shader, "mvp");
	assert(m_uniform_mvp >= 0);
	glcheck( glUniformMatrix4fv(m_uniform_mvp, 1, GL_FALSE, &(mvp.m[0][0])) );

	if (NULL != jointMatrices && jointMatrixCount > 0)
	{
		m_uniform_jointMatrix = glGetUniformLocation(_shader, "joint_matrices");
		if (m_uniform_jointMatrix >= 0)
		{
			glcheck( glUniformMatrix4fv(m_uniform_jointMatrix, jointMatrixCount, GL_FALSE, &(jointMatrices->x1)) );
		}
	}

	//if (m_matrix_changed)
	//	_update_mvp(shader);

	// texture
	uint tex = static_cast<uint>(reinterpret_cast<uint64>(texture));
	glcheck( glBindTexture(GL_TEXTURE_2D, tex) );

	//// buffer
	glcheck( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	static_cast<uint>(reinterpret_cast<uint64>(indexBuffer))) );

	//// vertex attribute 
	for (int i = 0; i < attrCount; ++i)
	{
		void* vertexBuffer = vertexBuffers[i];
		if (NULL == vertexBuffer)
			vertexBuffer = vertexBuffers[0];
		glcheck( glBindBuffer(GL_ARRAY_BUFFER, static_cast<uint>(reinterpret_cast<uint64>(vertexBuffer))) );

		VertexAttr a = attrs[i];
		if (a.location < 0)
			continue;
		glcheck( glEnableVertexAttribArray(a.location) );
		glcheck( glVertexAttribPointer(a.location, a.size, VertexAttrDataTypeToGLDataType(a.dataType), a.normalize, a.stride, (void*)a.offset) );
		//if (a.index >= 3)
		//{
		//		printf("index = %d size = %d type = %d stride = %d offset = %x\n", a.index, a.size, VertexAttrDataTypeToGLDataType(a.dataType), a.stride, a.offset);
		//}
	}

	glcheck( glDrawElements(primitiveType, indexCount, VertexAttrDataTypeToGLDataType((ELEM_TYPE)indexComponentType), reinterpret_cast<void*>(static_cast<uintptr_t>(indexOffset)) ) );

	//clear
	glcheck( glDisableVertexAttribArray(0) );
	glcheck( glDisableVertexAttribArray(1) );
	glcheck( glDisableVertexAttribArray(2) );
	glcheck( glBindVertexArray(0) );
	glcheck( glBindTexture(GL_TEXTURE_2D, 0) );
	glcheck( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	glcheck( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
}

void UIRenderOpenGL::_set_uniform(void* _shader, const char* const name, const uint color)
{
	const uint shader = static_cast<uint>(reinterpret_cast<uint64>(_shader));
	_use_program(shader);

	int loc = glGetUniformLocation(shader, name);
	assert(loc > 0);
	float a = 0, r = 0, g = 0, b = 0;
	argb_to_float(color, a, r, g, b);
	glcheck( glUniform4f(loc, r, g, b, a) );
}

int UIRenderOpenGL::getDeviceWidth() const
{
#if defined(__APPLE__)
	return m_width;
#else
	return m_eglWindow.width();
#endif
}

int UIRenderOpenGL::getDeviceHeight() const
{
#if defined(__APPLE__)
	return m_height;
#else
	return m_eglWindow.height();
#endif
}

void UIRenderOpenGL::saveTexture(void* texture, const char* const filename)
{
}

#ifdef __APPLE__
bool UIRenderOpenGL::init(const int width, const int height)
#else
bool UIRenderOpenGL::init(void* hInstance, void* hwnd)
#endif
{
	if (is_init())
		return false;

	// window surface
#if defined(__APPLE__)
	m_width		= width;
	m_height	= height;
#else
	m_eglWindow.create(hwnd);
#endif

	// mvp matrix
	//calcViewMatrix();

	// clear color
	glcheck( glClearColor(0.0f, 0.0f, 0.2f, 1.0f) );
	//glcheck( glClearColor(0.75f, 0.75f, 0.75f, 1.0f) );

	// view port
	glcheck( glViewport(0, 0, getDeviceWidth(), getDeviceHeight()) );

	m_init = true;

	return true;
}

void UIRenderOpenGL::swap()
{
#ifndef __APPLE__
	m_eglWindow.swap();
#endif
}

void UIRenderOpenGL::clear()
{
	glcheck( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
	glcheck( glEnable(GL_BLEND) );
	glEnable(GL_DEPTH_TEST);
	glcheck( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void UIRenderOpenGL::onResize(const int width, const int height, bool forceSet)
{
	if (!m_init)
		return;
	if (width == getDeviceWidth() && height == getDeviceHeight())
		return;

#ifdef __APPLE__
	m_width = width;
	m_height = height;
#else
	if (forceSet)
		m_eglWindow.update_size(width, height);
	else
		m_eglWindow.update_size();
#endif

	// mvp matrix
	//calcViewMatrix();

	// view port
	glcheck( glViewport(0, 0, getDeviceWidth(), getDeviceHeight()) );
}

void UIRenderOpenGL::copyIndexBuffer(const void* data, void* indexBuffer, const int sizeInByte)
{
	uint vbo = static_cast<uint>(reinterpret_cast<uint64>(indexBuffer));
	glcheck( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo) );
	glcheck( glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInByte, data, GL_STATIC_DRAW) );
	glcheck( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
}

void* UIRenderOpenGL::createShader(int shaderType)
{
	// shader 
	uint shader = 0;
	switch (shaderType)
	{
	case 0			: shader = cat::load_shader(cat::shader_vs_skin, cat::shader_ps				);	break;
	//case ui::SHADER_COLOR			: shader = cat::load_shader(shader_vs, shader_color_ps			);	break;
	//case ui::SHADER_FONT			: shader = cat::load_shader(shader_vs, shader_font_ps			);	break;
	//case ui::SHADER_FONT_OUTLINE	: shader = cat::load_shader(shader_vs, shader_font_outline_ps	);	break;
	//case ui::SHADER_ADD_COLOR		: shader = cat::load_shader(shader_vs, shader_add_color_ps		);	break;
	//case ui::SHADER_GRAY			: shader = cat::load_shader(shader_vs, shader_gray_ps			);	break;
	//case ui::SHADER_VIDEO			: shader = cat::load_shader(shader_vs, shader_ps				);	break;
	default : assert(false); break;
	}
	//if (shaderType == ui::SHADER_COLOR)
	//	m_effectShader = shader;
	return reinterpret_cast<void*>(static_cast<uintptr_t>(shader));
}

void* UIRenderOpenGL::createShader(const char* const vs_code, const char* const ps_code)
{
	// shader 
	uint shader = 0;
	shader = cat::load_shader(vs_code, ps_code);
	return reinterpret_cast<void*>(static_cast<uintptr_t>(shader));
}

void UIRenderOpenGL::useShader(void* _shader)
{
	const uint shader = static_cast<int>(reinterpret_cast<uint64>(_shader));
	_use_program(shader);
}

void UIRenderOpenGL::releaseShader(void* _shader)
{
	const uint shader = static_cast<int>(reinterpret_cast<uint64>(_shader));
	if (shader > 0)
		glDeleteProgram(shader);
}

void UIRenderOpenGL::_use_program(const uint shader)
{
	if (shader <= 0)
		return;

	GLint current_shader = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &current_shader);

	if (shader == current_shader)
		return;
	//m_shader = shader;
	glUseProgram(shader);
}

void UIRenderOpenGL::_set_texture_param()
{
	glcheck( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) );  
	glcheck( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) );  

	glcheck( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
	glcheck( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
}

void UIRenderOpenGL::release()
{
	//m_eglWindow.release();
}

UIRenderOpenGL::~UIRenderOpenGL()
{
#ifndef __APPLE__
	m_eglWindow.release();
#endif
}

void UIRenderOpenGL::scale(const float v)
{
	m_scale = v;
	//calcViewMatrix();
}


} //namespace cat


