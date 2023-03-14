#pragma once

#include "cat/string.h"

#include "scl/tree.h"

struct cgltf_primitive;

namespace cat {

class Shader;
class ShaderMacro;
class ShaderMacroArray;

class ShaderCache
{
public:
	~ShaderCache();

	Shader*			getShader					(const char* const vsFilename, const char* const fsFilename, const ShaderMacro* macros, int macroCount);
	Shader*			getShader					(const char* const vsFilename, const char* const fsFilename, cgltf_primitive* data, const int skinJointCount);
	Shader*			addMacro					(Shader* shader, const char* macro);
	Shader*			addMacro					(Shader* shader, const char** macros, const int macroCount);
	Shader*			removeMacro					(Shader* shader, const char* macro);
	Shader*			removeMacro					(Shader* shader, const char** macros, const int macroCount);
	Shader*			getDefaultShader			(const ShaderMacroArray& macros);
	Shader*			getPickShader				(Shader* shader);

private:
	enum MODIFY_TYPE
	{
		MODIFY_TYPE_ADD,
		MODIFY_TYPE_REMOVE,
	};
	Shader*			_modifyMacro				(Shader* shader, const char** macros, const int macroCount, const MODIFY_TYPE type);
	static void		_getShacroFromGltfPrimitive	(cgltf_primitive* data, const int skinJointCount, ShaderMacroArray& macros);

private:
	typedef scl::tree<String, Shader*> ShaderTree;
	ShaderTree m_shaders;
};

} // namespace cat

