
#include "cat/textureFile.h"
#include "cat/string.h"

#include "scl/tree.h"
#include "scl/vector.h"

struct cgltf_node;
struct cgltf_buffer_view;

namespace cat {

class Shader;
class ShaderCache;
class ShaderMacroArray;
class ShaderMacro;
class Material;
class Object;
class Primitive;
class IRender;

class Env
{
public:
	Env();
	~Env();

	IRender*			render					() { return m_render; }
	void				setRender				(IRender* v) { m_render = v; }
	Shader*				getShader				(const char* const vsFilename, const char* const fsFilename, const ShaderMacro* macros, const int macroCount);
	void				setDefaultShader		(const char* const vsFilename, const char* const fsFilename);
	Shader*				getDefaultShader		();
	Shader*				getDefaultShader		(const ShaderMacroArray& macros);
	ShaderCache*		shaderCache				() { return m_shaderCache; }
	void				setDefaultMaterial		(const char* const textureFilename);
	Material*			getDefaultMaterial		();
	const TextureFile*	getTextureFile			(const char* const filename);
	void				releaseTextureFile		(const TextureFile* textureFile);

	void				addToGltfNodeMap		(cgltf_node* node, int objectID);
	int					getObjectIDByGltfNode	(cgltf_node* node);
	Object*				getObjectByGltfNode		(cgltf_node* node);
	void				clearGltfNodeMap		();

	// pick primitive
	void				clearPickPrimtives		();
	scl::vector4		registerPickPrimitive	(Primitive* primitive);
	Primitive*			getPickPrimitive		(scl::vector4& color);

private:

	ShaderCache*								m_shaderCache;
	Shader*										m_defaultShader;
	IRender*									m_render;
	String										m_defaultMaterialTextureName;
	scl::tree<cgltf_node*, int>					m_gltfNodeMap;

	typedef scl::tree<string256, TextureFile>	TextureFileMap;
	TextureFileMap								m_textureFiles;

	//typedef scl::tree<uint32, Primitive*>		PickMap;
	typedef scl::array<Primitive*, 1024>		PickPrimitiveArray;
	PickPrimitiveArray							m_pickPrimitives;

}; // class Env

} // namespace cat