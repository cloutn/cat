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
#include "scl/log.h"

using scl::matrix;
using scl::vector3;

namespace cat {

ObjectIDMap<Object>* Object::s_objectIDMap			= NULL;
bool				 Object::s_objectIDMapReleased	= false;

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
	// 先把自己从父节点的 m_childs 里摘掉，避免外部 delete 中间节点后留悬垂指针
	if (NULL != m_parent)
	{
		m_parent->_removeChild(this);
		m_parent = NULL;
	}

	safe_delete(m_skin);
	safe_delete(m_mesh);
	safe_delete(m_transform);
	//safe_delete(m_matrixWithAnimation);

	// 递归 delete 子节点前先把它们的 m_parent 置 NULL，
	// 否则 child 析构时会反向去 erase 正在析构中的我，造成重复处理
	for (int i = 0; i < m_childs.size(); ++i)
	{
		if (NULL == m_childs[i])
			continue;
		m_childs[i]->m_parent = NULL;
		delete m_childs[i];
	}

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
		// 父链矩阵奇异（典型：某层 scale 含 0 / NaN），Release 下 assert 失效，
		// 这里用 log_warning 代替静默，让拾取/Gizmo/编辑路径出问题时看得见
		log_warning("Object[%d] parentGlobalMatrix is singular, fallback to identity", m_id);
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
	// release 是单向终点：再走到这里说明上层在 releaseObjectIDMap 之后还在 new/del Object，必须暴露
	assert(!s_objectIDMapReleased);
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
	safe_delete(s_objectIDMap);
	s_objectIDMapReleased = true;
}

Object* Object::childByName(const char* const objectName, bool recursive)
{
	for (int i = 0; i < m_childs.size(); ++i)
	{
		Object* object = m_childs[i];
		if (object->name() == objectName)
			return object;
		if (recursive)
		{
			Object* child = object->childByName(objectName, true);
			if (NULL != child)
				return child;
		}
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

void Object::_removeChild(Object* c)
{
	if (NULL == c)
		return;

	for (int i = 0; i < m_childs.size(); ++i)
	{
		if (m_childs[i] == c)
		{
			m_childs.erase_fast(i);
			return;
		}
	}
}


} // namespace cat {


