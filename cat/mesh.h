#pragma once

#include "cat/primitive.h"
#include "cat/string.h"

#include "scl/varray.h"

struct cgltf_mesh;

namespace cat {

class Material;
class Env;
class Object;

// A mesh is a collection of primitives
class Mesh
{
public:
	Mesh();
	~Mesh();

	void			load			(cgltf_mesh* mesh, const char* const path, int skinJointCount, Object* parent, IRender* render, Env* env);
	void			draw			(const scl::matrix& mvp, const scl::matrix* jointMatrices, const int jointMatrixCount, bool isPick, IRender* render);
	void			addPrimitive	(Primitive*);
	Object*			parent			() { return m_parent; }
	void			setParent		(Object* p) { m_parent = p ;}
	const String&	name			() const { return m_name; }
	void			setEnableSkin	(bool enable);
	Box				boundingBox		();

private:
	scl::varray<Primitive*>			m_primitives;
	String							m_name;

	//TODO parent is for debug
	Object*							m_parent;

	Env*							m_env;
};

}	// namespace cat


