#pragma once

#include "scl/vector.h"

namespace cat {

class Box
{
public:
	const scl::vector3&		center		() const					{ return m_center; }
	void					setCenter	(const scl::vector3& v)		{ m_center = v; }
	const scl::vector3&		extend		() const					{ return m_extend; }
	void					setExtend	(const scl::vector3& v)		{ m_extend = v; }

	enum class Type : uint8
	{
		MinMax,
		CenterExtend,
	};

private:
	union 
	{
		scl::vector3 m_center;
		scl::vector3 m_min;

	};
	union
	{
		scl::vector3 m_extend;
		scl::vector3 m_max;
	};
	Type m_type = Type::MinMax;
}; 



} // namesapce cat



