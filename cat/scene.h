#pragma once
////////////////////////////////////
// 2021.09.15 cloutn
////////////////////////////////////

#include "scl/varray.h"

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
	
	void						save			(const char* const filename);

	const scl::varray<Object*>&	objects			() const { return m_objects; }
	int							objectCount		() const { return m_objects.size(); }
	Object*						object			(const int index) { assert(index >= 0 && index < m_objects.size()); return m_objects[index]; }
	void						draw			(const scl::matrix& mvp, bool isPick);
	Object*						objectByName	(const char* const objectName, bool recursive = true);
	Object*						objectByID		(const int id, bool recursive = false);
	void						addObject		(Object* root) { m_objects.push_back(root); }
	void						setEnv			(Env* env) { m_env = env; }

private:
	Env*						m_env;
	scl::varray<Object*>		m_objects;


}; // class Scene



} // namespace cat




