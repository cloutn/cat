#include "cat/transform.h"

namespace cat {

Transform::Transform() : 
	m_move		({0, 0, 0}), 
	m_scale		({1, 1, 1}), 
	m_rotate	({0, 0, 0, 1}), 
	m_changed	(true),
	m_matrix	(scl::matrix::identity())
{

}

const scl::matrix& Transform::matrix() const
{
	if (!changed())
		return m_matrix;

	m_matrix = scl::matrix::identity();

	scl::matrix& self = m_matrix;

	//scale
	self.mul(scl::matrix::scale(m_scale.x, m_scale.y, m_scale.z));

	//rotate
	scl::matrix rotate;
	m_rotate.to_matrix(rotate);
	self.mul(rotate);

	//move
	self.mul(scl::matrix::move(m_move.x, m_move.y, m_move.z));

	/// ///move back to global coordinate system
	/// ///self.mul(scl::matrix::move(pivot.x, pivot.y, pivot.z));

	m_changed = false;

	return m_matrix;
}



} // namespace cat


