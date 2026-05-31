
#include "cat/mesh.h"

#include "cat/IRender.h"
#include "cat/material.h"
#include "cat/shaderMacro.h"
#include "cat/shaderCache.h"
#include "cat/shader.h"
#include "cat/env.h"

namespace cat {

Mesh::Mesh() : m_parent(NULL), m_env(NULL)
{

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
		if (b.isEmpty())
			continue;
		allBox += b;
	}
	return allBox;
}

} // namespace cat


