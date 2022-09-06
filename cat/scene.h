#pragma once
////////////////////////////////////
// 2021.09.15 cloutn
////////////////////////////////////

#include "scl/varray.h"

struct cgltf_scene;

namespace scl {
	class matrix;
}

namespace cat {

class Object;
class Env;
class IRender;

class Scene
{
public:
	Scene();
	~Scene();
	
	void						load		(cgltf_scene& scene, const char* const path, Env* env);
	void						save		(const char* const filename);

	const scl::varray<Object*>	objects		() const { return m_objects; }
	int							objectCount	() const { return m_objects.size(); }
	Object*						object		(const int index) { return m_objects[index]; }
	void						draw		(const scl::matrix& mvp, IRender* render);
	Object*						findObject	(const char* const objectName);

private:
	Env*						m_env;
	scl::varray<Object*>		m_objects;


}; // class Scene



} // namespace cat




