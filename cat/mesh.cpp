
#include "cat/mesh.h"

#include "cat/IRender.h"
#include "cat/material.h"
#include "cat/shaderMacro.h"
#include "cat/shaderCache.h"
#include "cat/shader.h"
#include "cat/env.h"

//#include "scl/assert.h"

#include "cgltf/cgltf.h"

namespace cat {

Mesh::Mesh() : m_parent(NULL), m_env(NULL)
{

}

void Mesh::load(cgltf_mesh* mesh, const char* const path, int skinJointCount, Object* parent, IRender* render, Env* env)
{
	assert(NULL != env);
	if (NULL == mesh || NULL == render || NULL == env)
		return;

	m_env = env;
	m_parent = parent;

	if (NULL != mesh->name)
	{
		m_name = mesh->name;
		//printf("\tloading mesh : %s\n", mesh->name);
	}

	for (size_t i = 0; i < mesh->primitives_count; ++i)
	{
		Primitive*			primitive		= new Primitive();
		cgltf_primitive*	primitiveNode	= &mesh->primitives[i];
		Shader*				shader			= m_env->shaderCache()->getShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", primitiveNode, skinJointCount); 
		primitive->setShaderWithPick(shader, env);
		primitive->load(primitiveNode, path, skinJointCount, this, render, env);

		m_primitives.push_back(primitive);
	}
}

void Mesh::draw(const scl::matrix& mvp, const scl::matrix* jointMatrices, const int jointMatrixCount, bool isPick, IRender* render)
{
	for (int i = 0; i < m_primitives.size(); ++i)
	{
		Primitive* primitive = m_primitives[i];
		if (NULL == primitive)
			continue;

		primitive->draw(mvp, jointMatrices, jointMatrixCount, isPick, render);
	}
}

Mesh::~Mesh()
{
	for (int i = 0; i < m_primitives.size(); ++i)
	{
		delete m_primitives[i];
	}
	m_primitives.clear();
}

void Mesh::addPrimitive(Primitive* p)
{
	m_primitives.push_back(p);
}


void Mesh::setEnableSkin(bool enable)
{
	for (int i = 0; i < m_primitives.size(); ++i)
	{
		Primitive* primitive = m_primitives[i];
		if (NULL == primitive)
			continue;

		Shader* newShader = NULL;
		if (enable)
			newShader = m_env->shaderCache()->addMacro(primitive->shader(), "SKIN");
		else
			newShader = m_env->shaderCache()->removeMacro(primitive->shader(), "SKIN");

		primitive->setShader(newShader);
	}
}


Box Mesh::boundingBox()
{
	Box allBox;
	for (int i = 0; i < m_primitives.size(); ++i)
	{
		Primitive* prim = m_primitives[i];
		if (NULL == prim)
			continue;
		Box b = prim->boundingBox();
		if (b.is_empty())
			continue;
		allBox += b;
	}
	return allBox;
}

} // namespace cat


