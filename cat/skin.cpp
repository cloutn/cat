
#include "cat/skin.h"

#include "cat/object.h"
#include "cat/env.h"
#include "cat/def.h"

#include "scl/matrix.h"
#include "scl/log.h"

#include <string.h>

using scl::matrix;

namespace cat {

Skin::Skin() : m_inverseBindMatrices(NULL), m_inverseBindMatrixCount(0), m_jointMatrices(NULL), m_root(NULL)
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
	m_inverseBindMatrixCount = count;
	if (NULL == matrices || count <= 0)
	{
		m_inverseBindMatrices = NULL;
		return;
	}
	m_inverseBindMatrices = new matrix[count];
	memcpy(m_inverseBindMatrices, matrices, sizeof(matrix) * count);
}

scl::matrix* Skin::generateJointMatrix(int& matrixCount, const scl::matrix& inverseMeshGlobalTransform)
{
	if (NULL == m_jointMatrices)
		m_jointMatrices = new matrix[m_inverseBindMatrixCount];

	for (int i = 0; i < m_inverseBindMatrixCount; ++i)
	{
		m_jointMatrices[i] = m_inverseBindMatrices[i];
		m_jointMatrices[i].mul(m_joints[i]->globalMatrix());
		m_jointMatrices[i].mul(inverseMeshGlobalTransform);
	}
	matrixCount = m_inverseBindMatrixCount;
	return m_jointMatrices;
}

cat::Object* Skin::root()
{
	if (NULL != m_root)
		return m_root;

	if (m_joints.size() <= 0)
		return NULL;

	// Start from first joint and trace up to find root
	Object* current = m_joints[0];
	const int MAX_DEPTH = m_joints.size() + 1;
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
	
	if (depth >= MAX_DEPTH && m_root == NULL)
	{
		m_root = m_joints[0];

		// TODO: Add warning log here if logging system is available
		log_warning("Possible cycle detected in joint hierarchy, using first joint as root");
	}
	
	return m_root;
}


} // namespace cat


