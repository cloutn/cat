#pragma once

#include "cat/transform.h"

#include "scl/vector.h"

namespace cat {

class Skybox
{
public:
	Skybox();

	scl::vector3	color		() const { return m_color; }
	void			setColor	(const scl::vector3& c) { m_color = c; }

	float			strength	() const { return m_strength; }
	void			setStrength	(const float v) { m_strength = v; }

private:
	Transform*		_transform	();

private:
	scl::vector3	m_color;
	float			m_strength;
	Transform*		m_transform;
};


} // namespace cat



