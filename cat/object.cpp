#include "cat/object.h"

#include "cat/IRender.h"
//#include "cat/camera.h"
#include "cat/material.h"
#include "cat/shader.h"
#include "cat/animation.h"
#include "cat/env.h"
//#include "cat/primitive.h"
#include "cat/mesh.h"
#include "cat/skin.h"

#include "gfx/vertex.h"
#include "gfx/base.h"

#include "scl/type.h"
#include "scl/assert.h"
#include "scl/vector.h"
#include "scl/file.h"
#include "scl/quaternion.h"

#include "cgltf/cgltf.h"

////// DEBUG //////
//#include "GLES3/gl3.h"
////// DEBUG //////

#include <map>

using scl::matrix;
using scl::vector3;

namespace cat {

ObjectIDMap<Object>* Object::s_objectIDMap = NULL;

Object::Object(Object* parent) :
	m_id			(_objectIDMap().alloc_id()),
	m_parent		(parent),
	//m_material		(NULL),
	//m_shader		(NULL),
	m_mesh			(NULL),
	m_skin			(NULL),
	m_move		(NULL),
	m_scale			(NULL),
	m_rotate		(NULL),
	m_loadMatrix	(NULL),
	m_matrix		(NULL),
	m_animationTransform		(NULL)
	//m_globalMatrix	(NULL)
	//m_inverseBindMatrix(NULL)
{
	_objectIDMap().add(this);
}

Object::~Object()
{
	//if (NULL != m_material)
	//{
	//	delete m_material;
	//	m_material = NULL;
	//}

	safe_delete(m_skin);
	safe_delete(m_mesh);

	for (int i = 0; i < m_childs.size(); ++i)
		delete m_childs[i];

	m_childs.clear();

	safe_delete(m_move);
	safe_delete(m_scale);
	safe_delete(m_rotate);
	safe_delete(m_matrix);
	//safe_delete(m_shader);
	//safe_delete(m_inverseBindMatrix);
	safe_delete(m_animationTransform);
	safe_delete(m_loadMatrix);



	_objectIDMap().del(this);
}

void Object::loadNode(cgltf_node* node, const char* const path, IRender* render, Env* env)
{
	m_env = env;
	env->addToGltfNodeMap(node, id());

	// create mesh
	assert(NULL == m_mesh);
	if (NULL != node->mesh)
	{
		m_mesh = new Mesh();
		m_mesh->load(node->mesh, path, node->skin == NULL ? 0 : node->skin->joints_count,  this, render, m_env);
	}
	
	if (NULL != node->name)
	{
		m_name = node->name;
		//printf("loading object: %s, parent = %s\n", node->name, NULL == m_parent ? "NULL" : m_parent->name().c_str());
	}

	//if (!om.count(node))
	//{
	//	om.add(node, this);
	//}
	//else
	//	assert(false);

	if (node->has_translation)
	{
		m_move = new scl::vector3 { node->translation[0], node->translation[1], node->translation[2] };
	}
	if (node->has_scale)
	{
		m_scale = new scl::vector3 { node->scale[0], node->scale[1], node->scale[2] };
	}
	if (node->has_rotation)
	{
		m_rotate = new scl::quaternion { node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3] };
	}
	if (node->has_matrix)
	{
		m_loadMatrix = new scl::matrix {
			node->matrix[0],	node->matrix[1],	node->matrix[2],	node->matrix[3],
			node->matrix[4],	node->matrix[5],	node->matrix[6],	node->matrix[7],
			node->matrix[8],	node->matrix[9],	node->matrix[10],	node->matrix[11],
			node->matrix[12],	node->matrix[13],	node->matrix[14],	node->matrix[15],
		};
	}

	// load childs
	for (size_t i = 0; i < node->children_count; ++i)
	{
		if (NULL == node->children[i])
			continue;

		Object* c = new Object(this);
		c->loadNode(node->children[i], path, render, m_env);
		m_childs.push_back(c);
	}
}

//void Object::init(const char* const filename, IRender* render, Env* env)
//{
//	m_env = env;
//
//	string256 path = filename;
//	scl::extract_path(path.pstring());
//
//	cgltf_data* data = gltf_load_from_file(filename);
//	if (NULL == data)
//		return;
//	if (NULL == data->scene && data->scenes_count == 0)
//		return;
//
//	//if (NULL == m_material)
//	//{
//	//	m_material = material; 
//	//}
//
//	//BufferMap bm;
//	//ObjectMap om;
//	for (int sceneIndex = 0; sceneIndex < (int)data->scenes_count; ++sceneIndex)
//	{
//		cgltf_scene& scene = data->scenes[sceneIndex];
//		for (size_t i = 0; i < scene.nodes_count; ++i)
//		{
//			assert(NULL != scene.nodes);
//			loadNode(scene.nodes[i], path.c_str(), render, m_env);
//		}
//		for (size_t i = 0; i < scene.nodes_count; ++i)
//		{
//			loadSkin(scene.nodes[i], env);
//		}
//	}
//
//	//m_animations.reserve(data->animations_count);
//	//for (cgltf_size i = 0; i < data->animations_count; ++i)
//	//{
//	//	cgltf_animation&	animationNode	= data->animations[i];
//	//	Animation*			animation		= new Animation();
//	//	animation->load(animationNode, env);
//	//	m_animations.push_back(animation);
//	//}
//
//	cgltf_free(data);
//}

void Object::draw(const scl::matrix& mvp, IRender* render)
{
	matrix*	jointMatrices		= NULL;
	int		jointMatrixCount	= 0;
	if (NULL != m_skin)
	{
		scl::matrix mat = globalMatrix();
		scl::matrix inverse;
		bool r = scl::matrix::inverse(mat, inverse);
		assert(r);
		jointMatrices = m_skin->generateJointMatrix(jointMatrixCount, inverse);
	}

	//////
	// TODO , whether if multiply root's inverse transform matrix.
	// in OpenGL, jointMatrix[i] = inverse(globalTransform) * globalJointTransform[i] * inverseBindMatrix[i];
	//
	//////

	if (NULL != m_mesh)
	{
		scl::matrix mat = globalMatrix();
		scl::matrix selfMvp = mat;
		selfMvp.mul(mvp);
		m_mesh->draw(selfMvp, jointMatrices, jointMatrixCount, render);
	}

	for (int i = 0; i < m_childs.size(); ++i)
	{
		if (NULL == m_childs[i])
			continue;
		m_childs[i]->draw(mvp, render);
	}
}

scl::matrix Object::globalMatrix()
{
	if (NULL == m_matrix)
	{
		m_matrix = new matrix();
	}

	if (m_name == "Mesh.001_0")
	{
		//printf("aaa\n");
	}

	if (NULL != m_loadMatrix && NULL == m_scale && NULL == m_rotate && NULL == m_move)
	{
		*m_matrix = *m_loadMatrix;
	}
	else
	{
		*m_matrix = matrix::identity();

		// scale
		if (NULL != m_scale)
			m_matrix->mul(matrix::scale(m_scale->x, m_scale->y, m_scale->z));

		// rotate
		if (NULL != m_rotate)
		{
			matrix matRotation;	
			m_rotate->to_matrix(matRotation);
			m_matrix->mul(matRotation);
		}

		// translate
		if (NULL != m_move)
			m_matrix->mul(matrix::move(m_move->x, m_move->y, m_move->z));

	}
	//if (NULL != m_animationTransform)
	//{
	//	const matrix& animationMatrix = m_animationTransform->matrix();	
	//	//if (m_name != "Bone_Armature")
	//	m_matrix->mul(animationMatrix);
	//}
	//else
	//{

	//}

	matrix result = *m_matrix;
	matrix parentMatrix = (NULL == m_parent) ? matrix::identity() : m_parent->globalMatrix();
	//if (*m_name == "Armature_rootJoint")
	//{
	//	parentMatrix = matrix::identity();
	//}
	//parentMatrix.mul(*m_matrix);
	result.mul(parentMatrix);
	return result;
}

void Object::loadSkin(cgltf_node* node, Env* env)
{
	if (NULL != node->skin)
	{
		assert(NULL == m_skin);
		m_skin = new Skin;
		m_skin->load(node->skin, env);
	}
	for (int i = 0; i < m_childs.size(); ++i)
	{
		m_childs[i]->loadSkin(node->children[i], env);
	}
}

ObjectIDMap<Object>& Object::_objectIDMap()
{
	if (NULL == s_objectIDMap)
	{
		s_objectIDMap = new ObjectIDMap<Object>;
		s_objectIDMap->init(MAX_OBJECT_COUNT);
	}
	return *s_objectIDMap;
}

void Object::setAnimationRotate(scl::quaternion& v)
{
	//_animationTransform()->setRotate(v);
	if (NULL == m_rotate)
		m_rotate = new scl::quaternion();
	*m_rotate = v;
}

void Object::setAnimationScale(scl::vector3 v)
{
	//_animationTransform()->setScale(v);
	if (NULL == m_scale)
		m_scale = new scl::vector3();
	*m_scale = v;
}

void Object::setAnimationMove(scl::vector3 v)
{
	//_animationTransform()->setMove(v);
	if (NULL == m_move)
		m_move = new scl::vector3();
	*m_move = v;
}

Transform* Object::_animationTransform()
{
	if (NULL == m_animationTransform)	
		m_animationTransform = new Transform;
	return m_animationTransform;
}

void Object::releaseObjectIDMap()
{
	if (NULL == s_objectIDMap)
		return;
	delete s_objectIDMap;
}

Object* Object::child(const char* const objectName)
{
	for (int i = 0; i < m_childs.size(); ++i)
	{
		Object* object = m_childs[i];
		if (object->name() == objectName)
			return object;
		Object* child = object->child(objectName);
		if (NULL != child)
			return child;
	}
	return NULL;
}

//void Object::setInverseBindMatrix(const scl::matrix& m)
//{
//	if (NULL != m_inverseBindMatrix)
//	{
//		assert(false);
//		return;
//	}
//	m_inverseBindMatrix = new matrix();	
//	*m_inverseBindMatrix = m;
//}

//const scl::matrix& Object::getInverseBindMatrix()
//{
//	if (NULL == m_inverseBindMatrix)
//		return scl::matrix::identity();
//	return *m_inverseBindMatrix;
//}

} // namespace cat {

