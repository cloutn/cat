#include "cat/shaderCache.h"

#include "cat/shader.h"
#include "cat/cgltf_util.h"

#include "scl/string.h"

#include "cgltf/cgltf.h"

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

cat::Shader* ShaderCache::getShader(const char* const vsFilename, const char* const fsFilename, cgltf_primitive* data, const int skinJointCount)
{
	ShaderMacroArray macros;
	_getShacroFromGltfPrimitive(data, skinJointCount, macros);

	Shader* shader = getShader(vsFilename, fsFilename, macros.data(), macros.size());
	return shader;
}

cat::Shader* ShaderCache::addMacro(Shader* shader, const char** addMacros, const int addMacroCount)
{
	Shader* newShader = _modifyMacro(shader, addMacros, addMacroCount, MODIFY_TYPE_ADD);
	return newShader;
}

cat::Shader* ShaderCache::addMacro(Shader* shader, const char* macro)
{
	const char* macros[] = { macro };
	Shader* newShader = addMacro(shader, macros, 1);
	return newShader;
}

cat::Shader* ShaderCache::removeMacro(Shader* shader, const char** removeMacros, const int removeMacroCount)
{
	Shader* newShader = _modifyMacro(shader, removeMacros, removeMacroCount, MODIFY_TYPE_REMOVE);
	return newShader;
}

cat::Shader* ShaderCache::removeMacro(Shader* shader, const char* macro)
{
	const char* macros[] = { macro };
	Shader* newShader = removeMacro(shader, macros, 1);
	return newShader;
}

cat::Shader* ShaderCache::getDefaultShader(const ShaderMacroArray& macros)
{
	return getShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros.data(), macros.size());
}

cat::Shader* ShaderCache::getPickShader(Shader* shader)
{
	ShaderMacroArray pickMacros;
	pickMacros.assign(shader->macros());
	pickMacros.remove("COLOR");
	pickMacros.remove("TEXTURE");
	pickMacros.add("PICK");
	Shader* pickShader = getShader(shader->vsFilename(), shader->psFilename(), pickMacros.data(), pickMacros.size());

	return pickShader;
}

cat::Shader* ShaderCache::_modifyMacro(Shader* shader, const char** macros, const int macroCount, const MODIFY_TYPE type)
{
	ShaderMacroArray newMacros;
	newMacros.assign(shader->macros());

	for (int i = 0; i < macroCount; ++i)
	{
		if (MODIFY_TYPE_ADD == type)
			newMacros.add(macros[i]);	
		else if (MODIFY_TYPE_REMOVE == type)
			newMacros.remove(macros[i]);	
	}

	Shader* newShader = getShader(shader->vsFilename(), shader->psFilename(), newMacros.data(), newMacros.size());
	return newShader;
}

void ShaderCache::_getShacroFromGltfPrimitive(cgltf_primitive* primitive, const int skinJointCount, ShaderMacroArray& macros)
{
	macros.clear();

	bool hasJoints	= cgltf_primitive_has_attr(primitive, "joints");
	bool hasWeights	= cgltf_primitive_has_attr(primitive, "weights");
	if (skinJointCount > 0 && hasJoints && hasWeights)
	{
		macros.add("SKIN");
		macros.add("JOINT_MATRIX_COUNT", skinJointCount);
	}
	if (cgltf_primitive_has_attr(primitive, "NORMAL"))
	{
		macros.add("NORMAL");
	}
	if (cgltf_primitive_has_attr(primitive, "TANGENT"))
	{
		macros.add("TANGENT");
	}
	if (cgltf_primitive_has_attr(primitive, "TEXCOORD"))
	{
		//if (NULL != m_material && NULL != m_material->texture())
		{
			macros.add("TEXTURE");
		}
		//else
		//{
		//	const char* objectName = (NULL == m_parent || NULL == m_parent->parent()) ? "" : m_parent->parent()->name().c_str();
		//	const char* meshName	= NULL == m_parent  ? "" : m_parent->name().c_str();
		//	printf("warning : object [%s] mesh [%s]\n\tprimitive attribute has TEXCOORD, but material has NO texture.\n", objectName, meshName);
		//}
	}
	if (cgltf_primitive_has_attr(primitive, "COLOR"))
	{
		macros.add("COLOR");
	}
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

