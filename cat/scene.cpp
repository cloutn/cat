#include "cat/scene.h"

#include "cat/object.h"
#include "cat/env.h"
#include "cat/yaml.h"

//#include "scl/ring_queue.h"


#include "cgltf/cgltf.h"

#include <queue>

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

void Scene::save(const char* const filename)
{
	yaml::document doc;
	yaml::node root = doc.root().set_map();

	root.add("name", "scene");

	yaml::node root_objects	= root.add_seq_inline("root_objects");
	yaml::node objects		= root.add_seq("objects");

	std::queue<Object*> queue;

	for (int i = 0; i < m_objects.size(); ++i)
	{
		if (NULL == m_objects[i])
			continue;

		queue.push(m_objects[i]);
		root_objects.add_val(i);
	}

	while (!queue.empty())
	{
		Object* object = queue.front();
		queue.pop();

		yaml::node objectNode = objects.add_map();
		objectNode.add("i", objects.child_count() - 1);
		object->save(objectNode);
		if (object->childCount() <= 0)
			continue;

		yaml::node objectChilds = objectNode.add_seq_inline("childs");
		int nextIndex = objects.child_count() + queue.size();
		for (int i = 0; i < object->childCount(); ++i)
		{
			objectChilds.add_val(nextIndex + i);
			queue.push(object->child(i));
		}
	}

	doc.save(filename);
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



