#pragma once

#include "cat/def.h"
#include "cat/string.h"

#include "scl/tree.h"
#include "scl/varray.h"

struct cgltf_data;
struct cgltf_scene;
struct cgltf_node;
struct cgltf_mesh;
struct cgltf_primitive;
struct cgltf_material;
struct cgltf_skin;
struct cgltf_animation;
struct cgltf_animation_channel;
struct cgltf_accessor;

namespace scl { class matrix; }

namespace cat {

class Env;
class IRender;
class Scene;
class Object;
class Mesh;
class Primitive;
class Material;
class Skin;
class Animation;
class AnimationChannel;
class Shader;
class VertexAttr;

// 把 .gltf 文件翻译成运行期 Scene/Object/Mesh/... 的加载器。
// 所有 cgltf 类型仅出现在 .cpp 与本文件中，runtime classes 不再含 cgltf 依赖。
class GltfLoader
{
public:
	GltfLoader();
	~GltfLoader();

	// 解析一个 gltf 文件，把结果追加到 outScenes / outAnimations。
	// 返回 false 表示加载失败（文件不存在 / 解析失败 / 无场景）。
	bool	loadFile	(const char* const filename, IRender* render, Env* env,
						 scl::varray<Scene*>& outScenes,
						 scl::varray<Animation*>& outAnimations);

private:
	void		_loadScene			(cgltf_scene& scene, Scene* outScene);
	Object*		_loadNode			(cgltf_node* node, Object* parent, int skinJointCount = 0);
	void		_loadSkinRecursive	(cgltf_node* node);
	void		_loadMesh			(cgltf_mesh* mesh, Object* host, int skinJointCount);
	void		_loadPrimitive		(cgltf_primitive* primitive, Mesh* host, int skinJointCount);
	void		_loadMaterial		(cgltf_material* material, Material* outMaterial);
	void		_loadSkin			(cgltf_skin* skinData, Skin* outSkin);
	Animation*	_loadAnimation		(cgltf_animation& animation);
	void		_loadAnimChannel	(const cgltf_animation_channel& channel, AnimationChannel* outChannel);

	// helpers (从 ShaderCache / Primitive / AnimationChannel 内的 cgltf 私有函数搬入)
	Shader*		_selectShader		(cgltf_primitive* src, int skinJointCount);
	byte*		_flattenVertexAttrs	(cgltf_primitive* src,
									 VertexAttr* outAttrs,
									 int& outAttrCount,
									 int& outVertexCount,
									 int& outStride);
	scl::matrix*_loadIBM				(cgltf_accessor* accessor, int outputCount);

	// cgltf_node* -> Object ID 反查表（取代 Env::m_gltfNodeMap）。
	int			_objectIDByNode		(cgltf_node* node) const;
	Object*		_objectByNode		(cgltf_node* node) const;
	void		_registerNode		(cgltf_node* node, int objectID);

private:
	IRender*								m_render;
	Env*									m_env;
	String									m_path;
	scl::tree<cgltf_node*, int>				m_nodeMap;

};	// class GltfLoader


} // namespace cat

