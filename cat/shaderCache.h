#pragma once

#include "cat/string.h"

#include "scl/tree.h"


namespace cat {

class Shader;
class ShaderMacro;

class ShaderCache
{
public:
	~ShaderCache();

	Shader* getShader(const char* const vsFilename, const char* const fsFilename, const ShaderMacro* macros, int macroCount);

private:
	//scl::tree<unsigned int, void*> m_deviceShaderMap;
	typedef scl::tree<String, Shader*> ShaderTree;
	ShaderTree m_shaders;
};

} // namespace cat

