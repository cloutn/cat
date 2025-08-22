#pragma once

#include "scl/matrix.h"
#include "scl/varray.h"

struct cgltf_skin;

namespace cat {

class Env;
class Object;

class Skin
{
public:
	Skin();
	virtual ~Skin();

	void					load				(cgltf_skin* skinData, Env* env);

	// TODO use cache, don't generate every frame
	scl::matrix*			generateJointMatrix	(int& matrixCount, const scl::matrix& inverseMeshGlobalTransform);

	const Object*			root				() const { return root(); }
	Object*					root				();

private:
	scl::matrix*			m_inverseBindMatrices;
	int						m_inverseBindMatrixCount;
	scl::varray<Object*>	m_joints;
	scl::matrix*			m_jointMatrices;
	mutable Object*			m_root;

};


} // namespace cat


