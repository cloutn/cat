
#include "cat/skin.h"

#include "cat/object.h"
#include "cat/env.h"
#include "cat/def.h"

#include "scl/matrix.h"

#include "cgltf\cgltf.h"

#include <string.h>

using scl::matrix;

namespace cat {

matrix* _loadMatrices(cgltf_accessor* accessor, int outputMatrixCount)
{
	if (NULL == accessor)
		return NULL;
	// only read float matrix 4x4
	if (accessor->component_type != cgltf_component_type_r_32f)
		return NULL;
	if (accessor->type != cgltf_type_mat4)
		return NULL;

	cgltf_buffer_view*	view		= accessor->buffer_view;
	const byte*			pBuffer		= (byte*)view->buffer->data + view->offset;
	if (view->size != sizeof(matrix) * outputMatrixCount)
		return NULL;
	matrix* output = new matrix[outputMatrixCount];
	memcpy(output, pBuffer, view->size);
	return output;
}

Skin::Skin() : m_inverseBindMatrices(NULL), m_inverseBindMatrixCount(0), m_jointMatrices(NULL)
{

}

Skin::~Skin()
{
	safe_delete_array(m_inverseBindMatrices);
	safe_delete_array(m_jointMatrices);
}

void Skin::load(cgltf_skin* skinData, Env* env)
{
	if (NULL == skinData)
		return;
	m_inverseBindMatrixCount = skinData->joints_count;
	m_inverseBindMatrices = _loadMatrices(skinData->inverse_bind_matrices, skinData->joints_count);

	m_joints.reserve(skinData->joints_count);
	for (int i = 0; i < static_cast<int>(skinData->joints_count); ++i)
	{
		cgltf_node* node = skinData->joints[i];
		Object* obj = env->getObjectByGltfNode(node);
		assert(NULL != obj);
		m_joints.push_back(obj);
	}
}

scl::matrix* Skin::generateJointMatrix(int& matrixCount, const scl::matrix& inverseMeshGlobalTransform)
{
	if (NULL == m_jointMatrices)
		m_jointMatrices = new matrix[m_inverseBindMatrixCount];

	for (int i = 0; i < m_inverseBindMatrixCount; ++i)
	{
		m_jointMatrices[i] = m_inverseBindMatrices[i];
		m_jointMatrices[i].mul(m_joints[i]->globalAnimationMatrix());
		m_jointMatrices[i].mul(inverseMeshGlobalTransform);
	}
	matrixCount = m_inverseBindMatrixCount;
	return m_jointMatrices;
}

} // namespace cat


