
#include "cat/box.h"
#include "scl/math.h"


namespace cat {

Box& Box::intersection(const Box& other)
{
	Box r = intersection(*this, other);
	*this = r;
	return *this;
}

Box Box::intersection(const Box& a, const Box& b)
{
	Box r;
	if (!a.intersect(b))
		return r;

	r.m_min.x = scl::max(a.m_min.x, b.m_min.x);
	r.m_min.y = scl::max(a.m_min.y, b.m_min.y);
	r.m_min.z = scl::max(a.m_min.z, b.m_min.z);

	r.m_max.x = scl::min(a.m_max.x, b.m_max.x);
	r.m_max.y = scl::min(a.m_max.y, b.m_max.y);
	r.m_max.z = scl::min(a.m_max.z, b.m_max.z);

	return r;
}

Box& Box::combine(const Box& other)
{
	Box r = combine(*this, other);
	*this = r;
	return *this;
}

Box Box::combine(const Box& a, const Box& b)
{
	Box r;

	r.m_min.x = scl::min(a.m_min.x, b.m_min.x);
	r.m_min.y = scl::min(a.m_min.y, b.m_min.y);
	r.m_min.z = scl::min(a.m_min.z, b.m_min.z);

	r.m_max.x = scl::max(a.m_max.x, b.m_max.x);
	r.m_max.y = scl::max(a.m_max.y, b.m_max.y);
	r.m_max.z = scl::max(a.m_max.z, b.m_max.z);

	return r;
}

bool Box::intersect(const Box& other) const
{
	if ((m_min.x > other.m_max.x) || (other.m_min.x > m_max.x))
		return false;

	if ((m_min.y > other.m_max.y) || (other.m_min.y > m_max.y))
		return false;

	if ((m_min.z > other.m_max.z) || (other.m_min.z > m_max.z))
		return false;

	return true;
}

void Box::ensureValid()
{
	if (m_min.x > m_max.x)
		scl::swap(m_min.x, m_max.x);

	if (m_min.y > m_max.y)
		scl::swap(m_min.y, m_max.y);

	if (m_min.z > m_max.z)
		scl::swap(m_min.z, m_max.z);
}

} // namespace cat
