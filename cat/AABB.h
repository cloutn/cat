#pragma once

#include "scl/vector.h"

namespace cat {


class AABB
{
public:
	const scl::vector3&		center		() const					{ return m_center; }
	void					setCenter	(const scl::vector3& v)		{ m_center = v; }
	const scl::vector3&		extend		() const					{ return m_extend; }
	void					setExtend	(const scl::vector3& v)		{ m_extend = v; }


private:
	scl::vector3 m_center;
	scl::vector3 m_extend; // from center to edge. size is 2 * m_extend.
}; 

class AABBMinMax
{
public:
	const scl::vector3&		min			() const					{ return m_min; }
	void					setMin		(const scl::vector3& v)		{ m_max = v; }
	const scl::vector3&		max			() const					{ return m_max; }
	void					setMax		(const scl::vector3& v)		{ m_max = v; }

private:
	scl::vector3 m_min;
	scl::vector3 m_max;
};


} // namesapce cat



