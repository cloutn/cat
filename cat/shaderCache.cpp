
#include "cat/shaderCache.h"
#include "cat/shader.h"

#include "scl/string.h"

namespace cat {


// BKDR Hash Function
//unsigned int BKDRHash(const char* str)
//{
//	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
//	unsigned int hash = 0;
//	while (*str)
//		hash = hash * seed + (*str++);
//	return (hash & 0x7FFFFFFF);
//}

Shader* ShaderCache::getShader(const char* const vsFilename, const char* const psFilename, const ShaderMacro* macros, int macroCount)
{
	String key;

	string256 strVsFilename = vsFilename;
	strVsFilename.trim();
	key += strVsFilename.c_str();
	key += ";";

	string256 strPsFilename = psFilename;
	strPsFilename.trim();
	key += strPsFilename.c_str();
	key += ";";

	for (int i = 0; i < macroCount; ++i)
	{
		String name = macros[i].name;
		name.trim();
		String value = macros[i].value;
		value.trim();
		//macros[i].name.trim();
		//macros[i].value.trim();
		key += name.c_str();
		key += ":";
		key += value.c_str();
		key += ":";
	}

	ShaderTree::iterator iter = m_shaders.find(key);
	if (iter != m_shaders.end())
	{
		return (*iter).second;
	}

	Shader* shader = new Shader();
	shader->load(strVsFilename.c_str(), strPsFilename.c_str());
	for (int i = 0; i < macroCount; ++i)
	{
		shader->addMacro(macros[i]);
	}
	m_shaders.add(key, shader);

	return shader;
}

ShaderCache::~ShaderCache()
{
	for (ShaderTree::iterator it = m_shaders.begin(); it != m_shaders.end(); ++it)
	{
		Shader* shader = (*it).second;
		delete shader;
	}
}

} // namespace cat

