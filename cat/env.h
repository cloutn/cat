
#include "cat/textureFile.h"
#include "cat/string.h"

#include "scl/tree.h"

struct cgltf_node;
struct cgltf_buffer_view;

namespace cat {

class Shader;
class ShaderCache;
class ShaderMacro;
class Material;
class Object;
class IRender;

class Env
{
public:
	Env();
	~Env();

	IRender*			render					() { return m_render; }
	void				setRender				(IRender* v) { m_render = v; }
	Shader*				getShader				(const char* const vsFilename, const char* const fsFilename, ShaderMacro* macros, const int macroCount);
	void				setDefaultShader		(const char* const vsFilename, const char* const fsFilename);
	Shader*				getDefaultShader		();
	void				setDefaultMaterial		(const char* const textureFilename);
	Material*			getDefaultMaterial		();
	const TextureFile*	getTextureFile			(const char* const filename);
	void				releaseTextureFile		(const TextureFile* textureFile);

	void				addToGltfNodeMap		(cgltf_node* node, int objectID);
	int					getObjectIDByGltfNode	(cgltf_node* node);
	Object*				getObjectByGltfNode		(cgltf_node* node);
	void				clearGltfNodeMap		();

private:
	ShaderCache*				m_shaderCache;
	Shader*						m_defaultShader;
	IRender*					m_render;
	String						m_defaultMaterialTextureName;
	scl::tree<cgltf_node*, int> m_gltfNodeMap;

	typedef scl::tree<string256, TextureFile> TextureFileMap;
	TextureFileMap m_textureFiles;

}; // class Env

} // namespace cat