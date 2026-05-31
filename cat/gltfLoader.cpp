#include "cat/gltfLoader.h"

#include "cat/animation.h"
#include "cat/animationChannel.h"
#include "cat/cgltf_util.h"
#include "cat/def.h"
#include "cat/env.h"
#include "cat/gltf_raw_render.h"
#include "cat/IRender.h"
#include "cat/keyFrame.h"
#include "cat/material.h"
#include "cat/mesh.h"
#include "cat/object.h"
#include "cat/primitive.h"
#include "cat/scene.h"
#include "cat/shader.h"
#include "cat/shaderCache.h"
#include "cat/shaderMacro.h"
#include "cat/skin.h"
#include "cat/vertex.h"

#include "scl/assert.h"
#include "scl/file.h"
#include "scl/log.h"
#include "scl/matrix.h"
#include "scl/quaternion.h"
#include "scl/string.h"
#include "scl/vector.h"

#include "cgltf/cgltf.h"

#include <string.h>

namespace cat {

using scl::matrix;
using scl::quaternion;
using scl::vector3;

namespace {

KEY_FRAME_TYPE _cgltfType2KeyFrameType(cgltf_animation_path_type ctype)
{
	switch (ctype)
	{
	case cgltf_animation_path_type_rotation		: return KEY_FRAME_TYPE_ROTATE;
	case cgltf_animation_path_type_scale		: return KEY_FRAME_TYPE_SCALE;
	case cgltf_animation_path_type_translation	: return KEY_FRAME_TYPE_MOVE;
	default: assert(false); break;
	};
	return KEY_FRAME_TYPE_INVALID;
}

}	// namespace

GltfLoader::GltfLoader() :
	m_render	(NULL),
	m_env		(NULL)
{
}

GltfLoader::~GltfLoader()
{
}

bool GltfLoader::loadFile(const char* const filename, IRender* render, Env* env,
						  scl::varray<Scene*>& outScenes,
						  scl::varray<Animation*>& outAnimations)
{
	if (NULL == filename || NULL == render || NULL == env)
	{
		assert(false);
		return false;
	}

	m_render	= render;
	m_env		= env;
	m_path		= filename;
	scl::extract_path(m_path.pstring());

	m_nodeMap.clear();

	cgltf_data* data = gltf_load_from_file(filename);
	if (NULL == data)
		return false;
	if (NULL == data->scene && data->scenes_count == 0)
	{
		cgltf_free(data);
		return false;
	}

	// scenes : pass 1 build object tree, pass 2 load skins
	for (size_t sceneIndex = 0; sceneIndex < data->scenes_count; ++sceneIndex)
	{
		cgltf_scene&	sceneNode	= data->scenes[sceneIndex];
		Scene*			scene		= new Scene();
		scene->setEnv(m_env);
		_loadScene(sceneNode, scene);
		outScenes.push_back(scene);
	}

	// set per-Object gltfIndex (= index into data->nodes[])
	for (size_t nodeIndex = 0; nodeIndex < data->nodes_count; ++nodeIndex)
	{
		cgltf_node*	node	= &data->nodes[nodeIndex];
		Object*		object	= _objectByNode(node);
		if (NULL == object)
		{
			assert(false);
			continue;
		}
		object->setGltfIndex(static_cast<int>(nodeIndex));
	}

	// animations
	for (cgltf_size i = 0; i < data->animations_count; ++i)
	{
		Animation* anim = _loadAnimation(data->animations[i]);
		if (NULL != anim)
			outAnimations.push_back(anim);
	}

	cgltf_free(data);
	return true;
}

void GltfLoader::_loadScene(cgltf_scene& scene, Scene* outScene)
{
	// pass 1 : build object tree, register all gltf_node -> Object
	for (size_t i = 0; i < scene.nodes_count; ++i)
	{
		cgltf_node* node = scene.nodes[i];
		if (NULL == node)
			continue;
		Object* root = _loadNode(node, NULL);
		outScene->addObject(root);
	}

	// pass 2 : load skins, recursively traversing gltf tree (bug fix:
	// 旧实现用 m_childs[i] 对位 node->children[i]，遇到 NULL child 会错位/崩溃)
	for (size_t i = 0; i < scene.nodes_count; ++i)
	{
		_loadSkinRecursive(scene.nodes[i]);
	}
}

Object* GltfLoader::_loadNode(cgltf_node* node, Object* parent, int /*unused*/)
{
	if (NULL == node)
		return NULL;

	Object* object = new Object(parent);
	_registerNode(node, object->id());

	const int skinJointCount = (NULL == node->skin) ? 0 : static_cast<int>(node->skin->joints_count);

	// mesh
	if (NULL != node->mesh)
		_loadMesh(node->mesh, object, skinJointCount);

	// name
	if (NULL != node->name)
		object->setName(node->name);

	// transform
	if (node->has_translation)
		object->setMove({ node->translation[0], node->translation[1], node->translation[2] });
	if (node->has_scale)
		object->setScale({ node->scale[0], node->scale[1], node->scale[2] });
	if (node->has_rotation)
		object->setRotate({ node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3] });
	if (node->has_matrix)
	{
		matrix m =
		{
			node->matrix[0],	node->matrix[1],	node->matrix[2],	node->matrix[3],
			node->matrix[4],	node->matrix[5],	node->matrix[6],	node->matrix[7],
			node->matrix[8],	node->matrix[9],	node->matrix[10],	node->matrix[11],
			node->matrix[12],	node->matrix[13],	node->matrix[14],	node->matrix[15],
		};
		object->setTransformByMatrix(m);
	}

	// children
	for (size_t i = 0; i < node->children_count; ++i)
	{
		cgltf_node* childNode = node->children[i];
		if (NULL == childNode)
			continue;
		Object* child = _loadNode(childNode, object);
		object->addChild(child);
	}

	return object;
}

void GltfLoader::_loadSkinRecursive(cgltf_node* node)
{
	if (NULL == node)
		return;

	Object* obj = _objectByNode(node);
	if (NULL != obj && NULL != node->skin)
	{
		assert(NULL == obj->skin());
		Skin* skin = new Skin();
		_loadSkin(node->skin, skin);
		obj->setSkin(skin);
		obj->setEnableSkin(true);
	}

	for (size_t i = 0; i < node->children_count; ++i)
		_loadSkinRecursive(node->children[i]);
}

void GltfLoader::_loadMesh(cgltf_mesh* mesh, Object* host, int skinJointCount)
{
	if (NULL == mesh)
		return;

	Mesh* m = new Mesh();
	m->setEnv(m_env);
	m->setParent(host);
	if (NULL != mesh->name)
		m->setName(mesh->name);

	for (size_t i = 0; i < mesh->primitives_count; ++i)
		_loadPrimitive(&mesh->primitives[i], m, skinJointCount);

	host->setMesh(m);
}

void GltfLoader::_loadPrimitive(cgltf_primitive* data, Mesh* host, int skinJointCount)
{
	if (NULL == data)
		return;

	Primitive* primitive = new Primitive();
	primitive->setRender(m_render);
	primitive->setEnv(m_env);
	primitive->setParent(host);
	primitive->setPrimitiveType(static_cast<PRIMITIVE_TYPE>(data->type));

	// shader
	Shader* shader = _selectShader(data, skinJointCount);
	primitive->setShaderWithPick(shader, m_env);

	// index
	const cgltf_accessor*	indices	= data->indices;
	if (NULL != indices)
	{
		const byte* pIndexBuffer = cgltf_get_accessor_buffer(indices);
		primitive->setIndices(pIndexBuffer, static_cast<int>(indices->count),
							  gltf_type_to_attr_type(indices->component_type));
	}

	// vertex: interleave all attrs into one buffer, all attrs share buffer index 0
	int		attrCount		= 0;
	int		vertexCount		= 0;
	int		stride			= 0;
	const int MAX_ATTR		= 32;
	VertexAttr	attrs[MAX_ATTR];
	byte*	buffer			= _flattenVertexAttrs(data, attrs, attrCount, vertexCount, stride);
	if (NULL != buffer && attrCount > 0)
	{
		primitive->setAttrs(attrs, attrCount, NULL);	// all share buffer index 0
		primitive->setVertices(buffer, vertexCount, stride);
		delete[] buffer;
	}

	// material
	if (NULL != data->material)
	{
		Material* mat = new Material();
		_loadMaterial(data->material, mat);
		primitive->setMaterial(mat);
	}
	else
	{
		primitive->setMaterial(m_env->getDefaultMaterial());
	}

	host->addPrimitive(primitive);
}

void GltfLoader::_loadMaterial(cgltf_material* data, Material* outMaterial)
{
	if (NULL == data || NULL == outMaterial)
		return;

	cgltf_texture* texture = NULL;
	if (data->has_pbr_metallic_roughness)
		texture = data->pbr_metallic_roughness.base_color_texture.texture;
	else if (data->has_pbr_specular_glossiness)
		texture = data->pbr_specular_glossiness.diffuse_texture.texture;
	else
		texture = data->normal_texture.texture;

	if (NULL == texture || NULL == texture->image || NULL == texture->image->uri)
		return;

	scl::string256 fullPath = m_path.c_str();
	fullPath += texture->image->uri;
	outMaterial->init(m_render, fullPath.c_str(), m_env);
}

void GltfLoader::_loadSkin(cgltf_skin* skinData, Skin* outSkin)
{
	if (NULL == skinData || NULL == outSkin)
		return;

	const int jointCount = static_cast<int>(skinData->joints_count);

	// inverse bind matrices
	matrix* ibm = _loadIBM(skinData->inverse_bind_matrices, jointCount);
	if (NULL == ibm)
	{
		// 不能传 (NULL, jointCount)，否则 Skin 内 count>0 / ptr==NULL 失配
		log_warning("Skin: inverse bind matrices missing/invalid, skin will be unusable");
		outSkin->setInverseBindMatrices(NULL, 0);
	}
	else
	{
		outSkin->setInverseBindMatrices(ibm, jointCount);
		safe_delete_array(ibm);
	}

	// joints
	for (int i = 0; i < jointCount; ++i)
	{
		cgltf_node* node = skinData->joints[i];
		Object*		obj	 = _objectByNode(node);
		if (NULL == obj)
		{
			log_warning("Skin: joint[%d] not mapped to Object, skipped", i);
			continue;
		}
		outSkin->addJoint(obj);
	}

	// optional explicit skeleton root
	if (NULL != skinData->skeleton)
	{
		Object* skeletonRoot = _objectByNode(skinData->skeleton);
		if (NULL == skeletonRoot)
			log_warning("Skin: skeleton root not mapped to Object, fallback to auto-resolve");
		else
			outSkin->setRoot(skeletonRoot);
	}
}

Animation* GltfLoader::_loadAnimation(cgltf_animation& animation)
{
	Animation* anim = new Animation();
	for (cgltf_size i = 0; i < animation.channels_count; ++i)
	{
		AnimationChannel* channel = new AnimationChannel();
		// 加载失败的 channel 必须丢弃，否则会作为半初始化对象残留在 m_channels，
		// 即使后续 update/apply 不崩也是无效遍历项，潜在隐患
		if (!_loadAnimChannel(animation.channels[i], channel))
		{
			safe_delete(channel);
			continue;
		}
		anim->addChannel(channel);
	}
	return anim;
}

bool GltfLoader::_loadAnimChannel(const cgltf_animation_channel& channel, AnimationChannel* outChannel)
{
	if (NULL == outChannel)
		return false;

	outChannel->setTarget(_objectIDByNode(channel.target_node));
	outChannel->setType(_cgltfType2KeyFrameType(channel.target_path));

	const cgltf_animation_sampler*	sampler			= channel.sampler;
	if (NULL == sampler || NULL == sampler->input || NULL == sampler->output)
	{
		assert(false);
		return false;
	}
	const cgltf_accessor*			timeAccessor	= sampler->input;
	const cgltf_accessor*			frameAccessor	= sampler->output;

	// time / frame 两个 accessor 必须等长，否则后面 for(frameCount) 会越读 frameDatas
	if (timeAccessor->count != frameAccessor->count)
	{
		log_error("AnimationChannel::loadKeyFrames count mismatch: time=%zu, frame=%zu",
			timeAccessor->count, frameAccessor->count);
		assert(false);
		return false;
	}
	const int						frameCount		= static_cast<int>(timeAccessor->count);

	if (timeAccessor->component_type != cgltf_component_type_r_32f ||
		timeAccessor->type != cgltf_type_scalar)
	{
		assert(false);
		return false;
	}
	if (frameAccessor->component_type != cgltf_component_type_r_32f ||
		(frameAccessor->type != cgltf_type_vec4 && frameAccessor->type != cgltf_type_vec3))
	{
		assert(false);
		return false;
	}

	const float*		times			= reinterpret_cast<const float*>(cgltf_get_accessor_buffer(timeAccessor));
	const float*		frameDatas		= reinterpret_cast<const float*>(cgltf_get_accessor_buffer(frameAccessor));
	// cgltf 不强制 buffer 已加载（外部 .bin 缺失 / cgltf_load_buffers 未调用都会返回 NULL），
	// 直接解引用必段错误；这里显式拦截
	if (NULL == times || NULL == frameDatas)
	{
		log_error("AnimationChannel::loadKeyFrames buffer not loaded (times=%p, frames=%p)", times, frameDatas);
		assert(false);
		return false;
	}

	const int			componentCount	= static_cast<int>(cgltf_num_components(frameAccessor->type));
	const KEY_FRAME_TYPE	type		= _cgltfType2KeyFrameType(channel.target_path);

	for (int i = 0; i < frameCount; ++i)
	{
		// NaN 自比较为 false，负值也按 0 兜底；避免 float → uint 是 UB
		const float		rawTime		= times[i];
		const float		safeTime	= (rawTime > 0.0f) ? rawTime : 0.0f;
		KeyFrame*		frame		= new KeyFrame(static_cast<uint>(safeTime * 1000));
		const float*	f			= &frameDatas[i * componentCount];

		switch (type)
		{
		case KEY_FRAME_TYPE_ROTATE:
			assert(componentCount >= 4);
			frame->setRotate(quaternion{ f[0], f[1], f[2], f[3] });
			break;
		case KEY_FRAME_TYPE_SCALE:
			frame->setScale(vector3{ f[0], f[1], f[2] });
			break;
		case KEY_FRAME_TYPE_MOVE:
			frame->setMove(vector3{ f[0], f[1], f[2] });
			break;
		default:
			assert(false);
			break;
		}
		outChannel->addKeyFrame(frame);
	}
	return true;
}

Shader* GltfLoader::_selectShader(cgltf_primitive* primitive, int skinJointCount)
{
	ShaderMacroArray macros;
	const bool hasJoints	= cgltf_primitive_has_attr(primitive, "joints");
	const bool hasWeights	= cgltf_primitive_has_attr(primitive, "weights");
	if (skinJointCount > 0 && hasJoints && hasWeights)
	{
		macros.add("SKIN");
		macros.add("JOINT_MATRIX_COUNT", skinJointCount);
	}
	if (cgltf_primitive_has_attr(primitive, "NORMAL"))
		macros.add("NORMAL");
	if (cgltf_primitive_has_attr(primitive, "TANGENT"))
		macros.add("TANGENT");
	if (cgltf_primitive_has_attr(primitive, "TEXCOORD"))
		macros.add("TEXTURE");
	if (cgltf_primitive_has_attr(primitive, "COLOR"))
		macros.add("COLOR");

	return m_env->shaderCache()->getShader(
		SHADER_PATH "object.vert",
		SHADER_PATH "object.frag",
		macros.data(),
		macros.size());
}

byte* GltfLoader::_flattenVertexAttrs(cgltf_primitive* data,
									  VertexAttr* outAttrs,
									  int& outAttrCount,
									  int& outVertexCount,
									  int& outStride)
{
	outAttrCount	= 0;
	outVertexCount	= 0;
	outStride		= 0;

	if (NULL == data || data->attributes_count <= 0)
		return NULL;

	// attributes[0].data 是后面访问 vertexCount 的前置条件
	if (NULL == data->attributes[0].data)
	{
		assert(false);
		return NULL;
	}
	const int	attrCount	= static_cast<int>(data->attributes_count);
	const int	vertexCount	= static_cast<int>(data->attributes[0].data->count);

	int		attrOffsets[1024]	= { 0 };
	int		sizeofVertex		= 0;
	for (int i = 0; i < attrCount; ++i)
	{
		cgltf_attribute&	attr		= data->attributes[i];
		cgltf_accessor*		accessor	= attr.data;
		const int			size		= static_cast<int>(cgltf_calc_size(accessor->type, accessor->component_type));
		sizeofVertex		+= size;
		attrOffsets[i + 1]	= attrOffsets[i] + size;
	}

	const int	bufferSize	= sizeofVertex * vertexCount;
	byte*		buffer		= new byte[bufferSize];
	memset(buffer, 0, bufferSize);

	for (int i = 0; i < attrCount; ++i)
	{
		cgltf_attribute&	attr		= data->attributes[i];
		cgltf_accessor*		accessor	= attr.data;

		outAttrs[i].location	= gltfAttrNameToLocation(attr.name);
		if (outAttrs[i].location < 0)
		{
			printf("attr index = %s\n", attr.name);
			continue;
		}
		outAttrs[i].size		= static_cast<int>(cgltf_num_components(accessor->type));
		outAttrs[i].dataType	= static_cast<ELEM_TYPE>(gltf_type_to_attr_type(accessor->component_type));
		outAttrs[i].normalize	= accessor->normalized;
		outAttrs[i].stride		= sizeofVertex;
		outAttrs[i].offset		= reinterpret_cast<void*>(static_cast<uintptr_t>(attrOffsets[i]));

		int elementSize = static_cast<int>(accessor->stride);
		if (elementSize == 0)
			elementSize = static_cast<int>(cgltf_calc_size(accessor->type, accessor->component_type));

		const byte* viewBuffer = cgltf_get_accessor_buffer(accessor);
		for (int vi = 0; vi < vertexCount; ++vi)
		{
			assert(vi * sizeofVertex + attrOffsets[i] + elementSize <= bufferSize);
			byte*		dst = buffer + vi * sizeofVertex + attrOffsets[i];
			const byte*	src = viewBuffer + vi * elementSize;
			memcpy(dst, src, elementSize);
		}
	}

	outAttrCount	= attrCount;
	outVertexCount	= vertexCount;
	outStride		= sizeofVertex;
	return buffer;
}

// IBM = Inverse Bind Matrices，gltf skin 里每根骨头一份的"逆绑定矩阵"。
// 作用：把顶点从模型空间搬到该骨头"绑定姿势(bind pose)下的局部空间"，
// 之后再乘上骨头当前帧的 global 矩阵，就得到顶点在当前动画姿势下的位置。
// 蒙皮最终矩阵：M_joint = inv(mesh.global) * joint.global * IBM
// 见 Skin::generateJointMatrix。gltf 里以 mat4 数组存在 skin.inverseBindMatrices accessor。
// 数量必须与 joints 数量一致，缺失/损坏时整个 skin 不可用。
scl::matrix* GltfLoader::_loadIBM(cgltf_accessor* accessor, int outputCount)
{
	if (NULL == accessor || outputCount <= 0)
		return NULL;
	if (accessor->component_type != cgltf_component_type_r_32f)
		return NULL;
	if (accessor->type != cgltf_type_mat4)
		return NULL;

	cgltf_buffer_view*	view	= accessor->buffer_view;
	if (NULL == view || NULL == view->buffer || NULL == view->buffer->data)
		return NULL;
	if (view->size != sizeof(matrix) * outputCount)
		return NULL;

	const byte*	pBuffer	= reinterpret_cast<const byte*>(view->buffer->data) + view->offset;
	matrix*		output	= new matrix[outputCount];
	memcpy(output, pBuffer, view->size);
	return output;
}

void GltfLoader::_registerNode(cgltf_node* node, int objectID)
{
	scl::tree<cgltf_node*, int>::iterator it = m_nodeMap.find(node);
	if (it != m_nodeMap.end())
	{
		assert(false);
		return;
	}
	m_nodeMap.add(node, objectID);
}

int GltfLoader::_objectIDByNode(cgltf_node* node) const
{
	scl::tree<cgltf_node*, int>::iterator it = m_nodeMap.find(node);
	if (it == m_nodeMap.end())
		return -1;
	return (*it).second;
}

Object* GltfLoader::_objectByNode(cgltf_node* node) const
{
	int id = _objectIDByNode(node);
	return Object::objectByID(id);
}


} // namespace cat

