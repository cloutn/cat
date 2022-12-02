#include "cat/object.h"

#include "cat/IRender.h"
#include "cat/material.h"
#include "cat/shader.h"
#include "cat/animation.h"
#include "cat/env.h"
#include "cat/mesh.h"
#include "cat/skin.h"
#include "cat/yaml.h"

#include "scl/type.h"
#include "scl/assert.h"
#include "scl/vector.h"
#include "scl/file.h"
#include "scl/quaternion.h"

#include "cgltf/cgltf.h"

using scl::matrix;
using scl::vector3;

namespace cat {

ObjectIDMap<Object>* Object::s_objectIDMap = NULL;

Object::Object(Object* parent) :
	m_id					(_objectIDMap().alloc_id()),
	m_parent				(parent),
	m_mesh					(NULL),
	m_skin					(NULL),
	m_matrixWithAnimation	(NULL),
	m_transform				(NULL),
	m_animationTransform	(NULL)
{
	_objectIDMap().add(this);
}

Object::~Object()
{
	safe_delete(m_skin);
	safe_delete(m_mesh);
	safe_delete(m_animationTransform);
	safe_delete(m_transform);
	safe_delete(m_matrixWithAnimation);

	for (int i = 0; i < m_childs.size(); ++i)
		delete m_childs[i];

	m_childs.clear();

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
		_animationTransform()->setMove({ node->translation[0], node->translation[1], node->translation[2] });
	}
	if (node->has_scale)
	{
		_animationTransform()->setScale({ node->scale[0], node->scale[1], node->scale[2] });
	}
	if (node->has_rotation)
	{
		_animationTransform()->setRotate({ node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3] });
	}
	if (node->has_matrix)
	{
		_animationTransform()->setByMatrix(
			{
				node->matrix[0],	node->matrix[1],	node->matrix[2],	node->matrix[3],
				node->matrix[4],	node->matrix[5],	node->matrix[6],	node->matrix[7],
				node->matrix[8],	node->matrix[9],	node->matrix[10],	node->matrix[11],
				node->matrix[12],	node->matrix[13],	node->matrix[14],	node->matrix[15],
			});
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

void Object::draw(const scl::matrix& mvp, bool isPick, IRender* render)
{
	scl::matrix*	jointMatrices		= NULL;
	int				jointMatrixCount	= 0;
	if (NULL != m_skin)
	{
		scl::matrix mat = globalAnimationMatrix();
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
		scl::matrix mat = globalMatrixWithAnimation();
		scl::matrix selfMvp = mat;
		selfMvp.mul(mvp);
		m_mesh->draw(selfMvp, jointMatrices, jointMatrixCount, isPick, render);
	}

	for (int i = 0; i < m_childs.size(); ++i)
	{
		if (NULL == m_childs[i])
			continue;
		m_childs[i]->draw(mvp, isPick, render);
	}
}

const scl::matrix& Object::matrix()
{
	const scl::matrix& mat = _transform()->matrix();
	return mat;
}

scl::matrix Object::globalMatrix()
{
	scl::matrix result = matrix();
	scl::matrix parentMatrix = (NULL == m_parent) ? scl::matrix::identity() : m_parent->globalMatrix();
	result.mul(parentMatrix);
	return result;
}

const scl::matrix& Object::matrixWithAnimation()
{
	if (NULL == m_matrixWithAnimation)
		m_matrixWithAnimation = new scl::matrix();

	//if (_transform()->changed() || _animationTransform()->changed())
	{
		*m_matrixWithAnimation = _animationTransform()->matrix();
		//*m_matrixWithAnimation = scl::matrix::identity();
		const scl::matrix& animationMatrix = _transform()->matrix();
		m_matrixWithAnimation->mul(animationMatrix);
	}
	return *m_matrixWithAnimation;
}

scl::matrix Object::globalMatrixWithAnimation()
{
	scl::matrix result = matrixWithAnimation();
	scl::matrix parentMatrix = (NULL == m_parent) ? scl::matrix::identity() : m_parent->globalMatrixWithAnimation();
	result.mul(parentMatrix);
	return result;
}

const scl::matrix& Object::animationMatrix()
{
	const scl::matrix& mat = _animationTransform()->matrix();
	return mat;
}

scl::matrix Object::globalAnimationMatrix()
{
	scl::matrix result = animationMatrix();
	scl::matrix parentMatrix = (NULL == m_parent) ? scl::matrix::identity() : m_parent->globalAnimationMatrix();
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

void Object::save(yaml::node& root)
{
	//yaml::node root = parent["childs"].add_map();
	root.add("name", m_name.c_str());
	root.add("id", m_id);

	//if (NULL != m_move)
	//	root.add("move", *m_move);
	//if (NULL != m_scale)
	//	root.add("scale", *m_scale);

	//yaml::node childs = root.add_seq("childs");
	//int childCount = parent["childs"].child_count();
	//return root;

	//for (int i = 0; i < m_childs.size(); ++i)
	//{
	//	if (NULL == m_childs[i])
	//		continue;
	//	m_childs[i]->save(root);
	//}
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


void Object::setRotate(const scl::quaternion& v)
{
	_transform()->setRotate(v);
}

void Object::setScale(const scl::vector3& v)
{
	_transform()->setScale(v);
}

void Object::setMove(const scl::vector3& v)
{
	_transform()->setMove(v);
}

void Object::setAnimationRotate(const scl::quaternion& v)
{
	_animationTransform()->setRotate(v);
}

void Object::setAnimationScale(const scl::vector3& v)
{
	_animationTransform()->setScale(v);
}

void Object::setAnimationMove(const scl::vector3& v)
{
	_animationTransform()->setMove(v);
}

Transform* Object::_transform()
{
	if (NULL == m_transform)	
		m_transform = new Transform;
	return m_transform;
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

