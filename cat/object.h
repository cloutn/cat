#pragma once

#include "cat/objectIDMap.h"
#include "cat/transform.h"

#include "scl/type.h"
#include "scl/matrix.h"
#include "scl/varray.h"
#include "cat/string.h"

struct cgltf_data;
struct cgltf_node;
struct cgltf_mesh;

namespace yaml { class node; }

namespace cat {
	
class IRender;
class Material;
//class Primitive;
class Mesh;
class Skin;
class Shader;
class Env;
class Animation;

class Object 
{
public:
	Object(Object* parent);
	virtual ~Object();

	void						loadNode				(cgltf_node* node, const char* const path, IRender* render, Env* env);
	void						loadSkin				(cgltf_node* node, Env* env);
	void						save					(yaml::node& parent);
	void						draw					(const scl::matrix& mvp, IRender* render);
	scl::matrix					globalMatrix			();
	const String&				name					() const { return m_name; }
	void						setName					(const char* const name) { m_name = name; }
	int							id						() const { return m_id; }
	const scl::varray<Object*>	childs					() const { return m_childs; }
	int							childCount				() const { return m_childs.size(); }
	Object*						child					(int index) { return m_childs[index]; }
	const Object*				child_const				(int index) const { return m_childs[index]; }
	Object*						child					(const char* const objectName);

	// static 
	static Object*				objectByID				(const int id) { return _objectIDMap().get(id); }
	static void					releaseObjectIDMap		();

	void						setAnimationRotate		(scl::quaternion& v);
	void						setAnimationScale		(scl::vector3 v);
	void						setAnimationMove		(scl::vector3 v);

private:
	Transform*					_animationTransform();

private:
	static ObjectIDMap<Object>*	s_objectIDMap;		//a map from object id to pointer. 
	static ObjectIDMap<Object>&	_objectIDMap();

	int							m_id;
	Env*						m_env;
	Object*						m_parent;
	Mesh*						m_mesh;
	Skin*						m_skin;
	scl::varray<Object*>		m_childs;

	scl::vector3*				m_move;
	scl::vector3*				m_scale;
	scl::quaternion*			m_rotate;
	scl::matrix*				m_loadMatrix;

	scl::matrix*				m_matrix;

	Transform*					m_animationTransform;

	//scl::matrix*				m_globalMatrix;
	String						m_name;

};  // class Object 

} // namespace cat {


