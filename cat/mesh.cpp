
#include "cat/mesh.h"

#include "cat/IRender.h"
#include "cat/material.h"

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
		Primitive* primitive = new Primitive();
		//printf("\t\tloading primitive: %d, type = %d\n", i, mesh->primitives[i].type);
		primitive->load(&mesh->primitives[i], path, skinJointCount, this, render, env);
		m_primitives.push_back(primitive);
	}
}

void Mesh::draw(const scl::matrix& mvp, const scl::matrix* jointMatrices, const int jointMatrixCount, IRender* render)
{
	for (int i = 0; i < m_primitives.size(); ++i)
	{
		Primitive* primitive = m_primitives[i];
		if (NULL == primitive)
			continue;

		primitive->draw(mvp, jointMatrices, jointMatrixCount, render);
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


} // namespace cat

