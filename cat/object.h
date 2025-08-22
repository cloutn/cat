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
class Mesh;
class Skin;
class Shader;
class Env;
class Animation;

class Object 
{
public:
	Object();
	Object(Object* parent);
	virtual ~Object();

	void						loadNode					(cgltf_node* node, const char* const path, IRender* render, Env* env);
	void						loadSkin					(cgltf_node* node, Env* env);
	void						save						(yaml::node& parent);
	void						draw						(const scl::matrix& mvp, bool isPick, IRender* render);
	const scl::matrix&			matrix						();
	scl::matrix					globalMatrix				();
	scl::matrix					parentGlobalMatrix			();
	scl::matrix					parentGlobalMatrixInverse	();
	const String&				name						() const { return m_name; }
	void						setName						(const char* const name) { m_name = name; }
	int							id							() const { return m_id; }
	const scl::varray<Object*>&	childs						() const { return m_childs; }
	int							childCount					() const { return m_childs.size(); }
	Object*						child						(int index) { return m_childs[index]; }
	const Object*				child						(int index) const { return m_childs[index]; }
	Object*						child						(const char* const objectName);
	Object*						childByID					(const int id, bool recursive = false);
	Object*						parent						() { return m_parent; }
	const Object*				parent						() const { return m_parent; }
	void						setRotate					(const scl::quaternion& v);
	void						setRotateAngle				(const scl::vector3& v);
	void						setScale					(const scl::vector3& v);
	void						setMove						(const scl::vector3& v);
	void						setPosition					(const scl::vector3& v) { setMove(v); }
	scl::vector3				position					();	
	scl::vector3				scale						();
	scl::quaternion				rotate						();
	scl::vector3				rotateAngle					();
	scl::vector3				rotateRadian				();
	bool						isEnableSkin				() const { return m_enableSkin; }
	void						setEnableSkin				(const bool enable);
	int							gltfIndex					() const { return m_gltfIndex; }
	void						setGltfIndex				(const int v) { m_gltfIndex = v; }
	bool						isEnableAnimation			() const { return m_enableAnimation; }
	void						setEnableAnimation			(const bool v) { m_enableAnimation = v; }
	void						setMesh						(Mesh* mesh) { m_mesh = mesh; }
	const Skin*					skin						() const { return m_skin; }
	bool						hasSkin						() const { return skin() != NULL; }
	Object*						skinRoot					();
	const Object*				skinRoot					() const { return skinRoot(); }

	// static 
	static Object*				objectByID					(const int id) { return _objectIDMap().get(id); }
	static void					releaseObjectIDMap			();

private:
	Transform*					_transform					();

private:
	static ObjectIDMap<Object>*	s_objectIDMap;		//a map from object id to pointer. 
	static ObjectIDMap<Object>&	_objectIDMap();

	int							m_id;
	//Env*						m_env;
	Object*						m_parent;
	Mesh*						m_mesh;
	Skin*						m_skin;
	scl::varray<Object*>		m_childs;
	scl::matrix*				m_matrixWithAnimation;
	Transform*					m_transform;
	String						m_name;
	bool						m_enableSkin;
	int							m_gltfIndex;
	bool						m_enableAnimation;

};  // class Object 


} // namespace cat



