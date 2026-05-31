
#include "cat/textureFile.h"
#include "cat/string.h"

#include "scl/tree.h"
#include "scl/varray.h"
#include "scl/vector.h"

namespace cat {

class Shader;
class ShaderCache;
class ShaderMacroArray;
class ShaderMacro;
class Material;
class Object;
class Primitive;
class IRender;
//class VertexAttrMapper;

using scl::string256;

class Env
{
public:
	Env();
	~Env();

	IRender*					render						() { return m_render; }
	void						setRender					(IRender* v) { m_render = v; }
	Shader*						getShader					(const char* const vsFilename, const char* const fsFilename, const ShaderMacro* macros, const int macroCount);
	void						setDefaultShader			(const char* const vsFilename, const char* const fsFilename);
	Shader*						getDefaultShader			();
	Shader*						getDefaultShader			(const ShaderMacroArray& macros);
	ShaderCache*				shaderCache					() { return m_shaderCache; }
	void						setDefaultMaterial			(const char* const textureFilename);
	Material*					getDefaultMaterial			();
	const TextureFile*			getTextureFile				(const char* const filename);
	void						releaseTextureFile			(const TextureFile* textureFile);

	// pick primitive
	void						clearPickPrimitives			();
	scl::vector4				registerPickPrimitive		(Primitive* primitive);
	Primitive*					getPickPrimitive			(scl::vector4& color);

	//VertexAttrMapper*			vertexAttrMapper			(const int index = 0);

private:

	ShaderCache*											m_shaderCache;
	Shader*													m_defaultShader;
	IRender*												m_render;
	String													m_defaultMaterialTextureName;

	typedef scl::tree<string256, TextureFile>				TextureFileMap;
	TextureFileMap											m_textureFiles;

	//typedef scl::tree<uint32, Primitive*>					PickMap;
	// pick 表每次 _clickSelectObject 入口 clear 后重填，元素数 = 单次 pick 帧内可拾取 primitive 数。
	// 用 varray 自动扩容，避免原 scl::array<T, 1024> 编译期定长在中大场景溢出。
	typedef scl::varray<Primitive*>							PickPrimitiveArray;
	PickPrimitiveArray										m_pickPrimitives;

	//const static int										MAX_VERTEX_ATTR_MAPPER_COUNT = 32;
	//VertexAttrMapper*										m_vertexAttrMappers;

}; // class Env

} // namespace cat

