#pragma once

#include "scl/vector.h"
#include "scl/quaternion.h"

namespace cat {

class Transform
{
public:
	Transform();

	const scl::vector3&			move		() const { return m_move;	} 
	const scl::vector3&			scale		() const { return m_scale;	}
	const scl::quaternion&		rotate		() const { return m_rotate;	}

	const scl::matrix&			matrix		() const;

	void						setMove		(scl::vector3& v)		{ m_move = v; invalidate(); }
	void						setScale	(scl::vector3& v)		{ m_scale = v; invalidate(); }
	void						setRotate	(scl::quaternion& v)	{ m_rotate = v; invalidate(); }

	bool						changed		() const { return m_changed; }
	void						invalidate	() { m_changed = true; }

private:
	scl::vector3				m_move;	
	scl::vector3				m_scale;	
	scl::quaternion				m_rotate;

	mutable bool				m_changed;			//是否需要更新
	mutable scl::matrix			m_matrix;			//变化矩阵，包含了父窗口的变化	

}; // class Transform


} // namespace cat



