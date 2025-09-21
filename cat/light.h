#pragma once

#include "cat/transform.h"

#include "scl/vector.h"

namespace cat {

class Light
{
public:
	Light();

	scl::vector3	color		() const { return m_color; }
	void			setColor	(const scl::vector3& c) { m_color = c; }

private:
	Transform*		_transform	();

private:
	scl::vector3	m_color;
	Transform*		m_transform;
};

} // namespace cat


