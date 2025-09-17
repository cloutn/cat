#pragma once

#include "scl/vector.h"

namespace cat {

class Box
{
public:
	Box() : m_min(scl::vector3::zero()), m_max(scl::vector3::zero()) {}
	Box(const scl::vector3& _min, const scl::vector3& _max) : m_min(_min), m_max(_max) { ensureValid(); }

	void					set				(const scl::vector3& _min, const scl::vector3& _max) { m_min = _min; m_max = _max; ensureValid(); }
	const scl::vector3&		min				() const					{ return m_min; }
	const scl::vector3&		max				() const					{ return m_max; }
	void					setMinDirect	(const scl::vector3& v)		{ m_min = v; }		
	void					setMaxDirect	(const scl::vector3& v)		{ m_max = v; }	
	
	Box&					intersection	(const Box& box);		
	Box&					combine			(const Box& box);	
	static Box				intersection	(const Box& a, const Box& b);
	static Box				combine			(const Box& a, const Box& b);

	bool					intersect		(const Box& other) const;
	void					ensureValid		();

private:
	scl::vector3			m_min;
	scl::vector3			m_max;
}; 



} // namespace cat



