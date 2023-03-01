#pragma once

#include "cat/shaderMacro.h"
#include "cat/def.h"
#include "cat/string.h"

#include "scl/varray.h"

#ifdef TEST_VULKAN
#define SHADER_PATH "shader/vulkan/"
#else
#define SHADER_PATH "shader/opengles/"
#endif
namespace cat {

class IRender;

class Shader 
{
public:
	Shader	() : m_deviceShader(NULL), m_dirty(false), m_render(NULL) {}
	~Shader	();

public:

	void				load			(const char* const vs_filename, const char* const ps_filename);
	void				addMacro		(const ShaderMacro& macro);
	void*				shader			(IRender* render); // 为什么要通过 Render ？ 因为 OpengGL和directX load shader 的方式不同？
	void				invalidate		() { m_dirty = true; }
	ShaderMacroArray&	macros			() { return m_macros; }
	const char*			vsFilename		() const { return m_vsFilename.c_str(); }
	const char*			psFilename		() const { return m_psFilename.c_str(); }

private:
	static char*		_loadfile		(const char* const filename, const char* const macros);
	void				_allmacros		(String& output);

private:
	String								m_vsFilename;
	String								m_psFilename;
	//scl::varray<ShaderMacro>			m_macros;
	ShaderMacroArray					m_macros;
	void*								m_deviceShader;
	bool								m_dirty;
	IRender*							m_render;
};

} // namespace cat

