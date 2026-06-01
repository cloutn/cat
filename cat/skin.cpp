
#include "cat/skin.h"

#include "cat/object.h"
#include "cat/env.h"
#include "cat/def.h"

#include "scl/matrix.h"
#include "scl/log.h"

#include <string.h>

using scl::matrix;

namespace cat {

Skin::Skin() :
	m_inverseBindMatrices		(NULL),
	m_inverseBindMatrixCount	(0),
	m_jointMatrices				(NULL),
	m_root						(NULL)
{

}

Skin::~Skin()
{
	safe_delete_array(m_inverseBindMatrices);
	safe_delete_array(m_jointMatrices);
}

void Skin::setInverseBindMatrices(const scl::matrix* matrices, int count)
{
	safe_delete_array(m_inverseBindMatrices);
	// IBM 变了，旧 jointMatrices 缓存可能尺寸不匹配，一并丢弃，下次 generateJointMatrix lazy 重建
	safe_delete_array(m_jointMatrices);

	if (NULL == matrices || count <= 0)
	{
		m_inverseBindMatrices    = NULL;
		m_inverseBindMatrixCount = 0;
		return;
	}
	m_inverseBindMatrixCount = count;
	m_inverseBindMatrices    = new matrix[count];
	memcpy(m_inverseBindMatrices, matrices, sizeof(matrix) * count);
}

void Skin::addJoint(Object* j)
{
	// joints 数量变化会改变 generateJointMatrix 的 count，旧 jointMatrices 容量可能失配，
	// 与 setInverseBindMatrices 对齐：丢弃后由 generateJointMatrix lazy 重建
	safe_delete_array(m_jointMatrices);
	m_joints.push_back(j);
}

scl::matrix* Skin::generateJointMatrix(int& matrixCount, const scl::matrix& inverseMeshGlobalTransform)
{
	matrixCount = 0;

	// 不变量：count==0 ⟺ ptr==NULL；任一失配立刻早退，避免 hot path 解 NULL
	if (NULL == m_inverseBindMatrices || m_inverseBindMatrixCount <= 0)
		return NULL;

	const int jointCount = static_cast<int>(m_joints.size());
	if (jointCount <= 0)
		return NULL;

	if (jointCount != m_inverseBindMatrixCount)
	{
		log_error("Skin: joint count %d mismatches inverse bind matrix count %d, skin invalidated", jointCount, m_inverseBindMatrixCount);
		return NULL;
	}
	const int count = jointCount;

	// 任一 joint 为 NULL 说明 GLTF 加载链断了，整副骨架已经不可信；
	// 退化到不蒙皮渲染（返回 NULL），不要用 bindpose 兜底导致画出半 bindpose / 半动画的扭曲姿态
	for (int i = 0; i < count; ++i)
	{
		if (NULL == m_joints[i])
		{
			log_error("Skin: joint[%d] is NULL, skin invalidated", i);
			return NULL;
		}
	}

	// setInverseBindMatrices / addJoint 已在变化时丢弃 m_jointMatrices，
	// 这里只需按 NULL 判断 lazy 分配
	if (NULL == m_jointMatrices)
		m_jointMatrices = new matrix[count];

	for (int i = 0; i < count; ++i)
	{
		m_jointMatrices[i] = m_inverseBindMatrices[i];
		m_jointMatrices[i].mul(m_joints[i]->globalMatrix());
		m_jointMatrices[i].mul(inverseMeshGlobalTransform);
	}
	matrixCount = count;
	return m_jointMatrices;
}

cat::Object* Skin::root()
{
	if (NULL != m_root)
		return m_root;

	if (m_joints.size() <= 0)
		return NULL;

	// 找到第一个非 NULL joint 作为回溯起点，避免 NULL joint 入队后崩
	Object* current = NULL;
	for (size_t i = 0; i < m_joints.size(); ++i)
	{
		if (NULL != m_joints[i])
		{
			current = m_joints[i];
			break;
		}
	}
	if (NULL == current)
		return NULL;

	const int MAX_DEPTH = static_cast<int>(m_joints.size()) + 1;
	int depth = 0;

	while (depth < MAX_DEPTH)
	{
		const Object* parent = current->parent();
		
		// If parent is NULL, current is root
		if (NULL == parent)
		{
			m_root = current;
			break;
		}
		
		// If parent is not in joints array, current is root
		if (!m_joints.contains(parent))
		{
			m_root = current;
			break;
		}
		
		// Move up to parent
		current = const_cast<Object*>(parent);
		++depth;
	}
	
	if (depth >= MAX_DEPTH && NULL == m_root)
	{
		// 兜底用回溯起点（已保证非 NULL），不要用 m_joints[0]（可能是 NULL）
		m_root = current;
		log_warning("Possible cycle detected in joint hierarchy, using first joint as root");
	}
	
	return m_root;
}


} // namespace cat


