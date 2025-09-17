#pragma once

#include "scl/vector.h"

namespace cat {

class Box
{
public:
	Box() : m_min(scl::vector3::zero()), m_max(scl::vector3::zero()) {}
	Box(const scl::vector3& _min, const scl::vector3& _max) : m_min(_min), m_max(_max) { ensureValid(); }

	const scl::vector3&		min				() const					{ return m_min; }
	const scl::vector3&		max				() const					{ return m_max; }
	scl::vector3			center			() const					{ return (m_min + m_max) * 0.5f; }
	scl::vector3			size			() const					{ return m_max - m_min; }
	bool					is_empty		() const					{ return m_max == m_min; }

	void					set				(const scl::vector3& _min, const scl::vector3& _max) { m_min = _min; m_max = _max; ensureValid(); }
	void					setMinDirect	(const scl::vector3& v)		{ m_min = v; }		
	void					setMaxDirect	(const scl::vector3& v)		{ m_max = v; }
	
	Box&					intersection	(const Box& box);		
	Box&					combine			(const Box& box);	
	static Box				intersection	(const Box& a, const Box& b);
	static Box				combine			(const Box& a, const Box& b);
	Box&					encapsulate		(const scl::vector3& p);
	Box&					encapsulate		(const Box& box);

	bool					intersect		(const Box& other) const;
	void					ensureValid		();

	Box&					operator+=		(const scl::vector3& p) { encapsulate(p); return *this; }
	Box&					operator+=		(const Box& box) { encapsulate(box); return *this; }

private:
	scl::vector3			m_min;
	scl::vector3			m_max;
}; 



} // namespace cat



