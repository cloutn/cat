#pragma once

#include "scl/matrix.h"
#include "scl/varray.h"

namespace cat {

class Env;
class Object;

class Skin
{
public:
	Skin();
	virtual ~Skin();

	// TODO use cache, don't generate every frame
	scl::matrix*			generateJointMatrix	(int& matrixCount, const scl::matrix& inverseMeshGlobalTransform);

	const Object*			root				() const { return const_cast<Skin*>(this)->root(); }
	Object*					root				();
	void					setRoot				(Object* r) { m_root = r; }
	void					addJoint			(Object* j) { m_joints.push_back(j); }
	void					setInverseBindMatrices(const scl::matrix* matrices, int count);

private:
	scl::matrix*			m_inverseBindMatrices;
	int						m_inverseBindMatrixCount;
	scl::varray<Object*>	m_joints;
	scl::matrix*			m_jointMatrices;
	mutable Object*			m_root;

};


} // namespace cat


