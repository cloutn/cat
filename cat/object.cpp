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

using scl::matrix;
using scl::vector3;

namespace cat {

ObjectIDMap<Object>* Object::s_objectIDMap = NULL;

Object::Object() : Object(NULL)
{
}

Object::Object(Object* parent) :
	m_id					(_objectIDMap().alloc_id()),
	m_parent				(parent),
	m_mesh					(NULL),
	m_skin					(NULL),
	//m_matrixWithAnimation	(NULL),
	m_transform				(NULL),
	m_enableSkin			(false),
	m_gltfIndex				(-1),
	m_enableAnimation		(true)
{
	_objectIDMap().add(this);
}

Object::~Object()
{
	safe_delete(m_skin);
	safe_delete(m_mesh);
	safe_delete(m_transform);
	//safe_delete(m_matrixWithAnimation);

	for (int i = 0; i < m_childs.size(); ++i)
		delete m_childs[i];

	m_childs.clear();

	_objectIDMap().del(this);

}

void Object::draw(const scl::matrix& mvp, bool isPick, IRender* render)
{
	scl::matrix*	jointMatrices		= NULL;
	int				jointMatrixCount	= 0;
	if (NULL != m_skin && isEnableSkin())
	{
		scl::matrix mat = globalMatrix();
		scl::matrix inverse;
		bool r = scl::matrix::inverse(mat, inverse);
		assert(r);
		jointMatrices = m_skin->generateJointMatrix(jointMatrixCount, inverse);
	}

	if (NULL != m_mesh)
	{
		scl::matrix mat = globalMatrix();
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

scl::matrix Object::parentGlobalMatrix()
{
	if (NULL == m_parent)
		return scl::matrix::identity();

	return m_parent->globalMatrix();
}

scl::matrix Object::parentGlobalMatrixInverse()
{
	scl::matrix inverse;
	bool success = matrix::inverse(parentGlobalMatrix(), inverse);
	if (!success)
	{
		assert(false);
		return scl::matrix::identity();
	}
	return inverse;
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
	if (v == _transform()->rotate())
		return;
	_transform()->setRotate(v);
}

void Object::setRotateAngle(const scl::vector3& v)
{
	scl::quaternion q;
	q.from_euler_angle(v.x, v.y, v.z);
	if (q == _transform()->rotate())
		return;

	_transform()->setRotateAngle(v);
}

void Object::setScale(const scl::vector3& v)
{
	if (v == _transform()->scale())
		return;
	_transform()->setScale(v);
}

void Object::setMove(const scl::vector3& v)
{
	if (v == _transform()->move())
		return;
	_transform()->setMove(v);
}

void Object::setTransformByMatrix(const scl::matrix& m)
{
	_transform()->setByMatrix(m);
}

scl::vector3 Object::position()
{
	return _transform()->move();
}

scl::vector3 Object::scale()
{
	return _transform()->scale();
}

scl::quaternion Object::rotate()
{
	return _transform()->rotate();
}

scl::vector3 Object::rotateRadian()
{
	scl::quaternion q = rotate();
	scl::vector3	radian;
	q.to_euler_radian(radian);
	return radian;
}

void Object::setEnableSkin(const bool enable)
{
	if (enable == m_enableSkin)
		return;

	if (NULL == m_mesh)
		return;

	if (NULL == m_skin)
		return;

	m_enableSkin = enable;

	m_mesh->setEnableSkin(enable);
}

scl::vector3 Object::rotateAngle()
{
	scl::quaternion q = rotate();
	scl::vector3	angle;
	q.to_euler_angle(angle);
	return angle;
}

Transform* Object::_transform()
{
	if (NULL == m_transform)	
		m_transform = new Transform;
	return m_transform;
}

cat::Object* Object::skinRoot() 
{
	return (NULL == m_skin) ? NULL : m_skin->root();
}

cat::Box Object::boundingBox() const
{
	if (m_mesh != NULL)
		return m_mesh->boundingBox();
	else
		return Box();
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


Object* Object::childByID(const int id, bool recursive)
{
	if (id < 0)
		return NULL;

	for (int i = 0; i < m_childs.size(); ++i)
	{
		Object* object = m_childs[i];
		if (object->id() == id)
			return object;
		if (recursive)
		{
			Object* child = object->childByID(id, true);
			if (NULL != child)
				return child;
		}
	}
	return NULL;
}


} // namespace cat {


