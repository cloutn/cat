#include "cat/scene.h"

#include "cat/object.h"
#include "cat/env.h"

#include "cgltf/cgltf.h"

namespace cat {


Scene::Scene() : m_env(NULL)
{

}

Scene::~Scene()
{
	for (int i = 0; i < m_objects.size(); ++i)
		delete m_objects[i];
}

void Scene::load(cgltf_scene& scene, const char* const path, Env* env)
{
	m_env = env;

	for (size_t i = 0; i < scene.nodes_count; ++i)
	{
		assert(NULL != scene.nodes);
		Object*		object	= new Object(NULL);
		cgltf_node*	node	= scene.nodes[i];
		object->loadNode(node, path, m_env->render(), m_env);
		m_objects.push_back(object);
	}

	for (size_t i = 0; i < scene.nodes_count; ++i)
	{
		cgltf_node*	node	= scene.nodes[i];
		Object* object = m_env->getObjectByGltfNode(node);		
		object->loadSkin(node, env);
	}
}

void Scene::draw(const scl::matrix& mvp, IRender* render)
{
	for (int i = 0; i < m_objects.size(); ++i)
		m_objects[i]->draw(mvp, render);
}

Object* Scene::findObject(const char* const objectName)
{
	for (int i = 0; i < m_objects.size(); ++i)
	{
		Object* object = m_objects[i];
		if (object->name() == objectName)
			return object;
		Object* child = object->child(objectName);
		if (NULL != child)
			return child;
	}
	return NULL;
}

} // namespace cat



