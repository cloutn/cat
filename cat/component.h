#pragma once

#include "cat/def.h"

#include "scl/varray.h"

namespace cat {

class Component
{
public:


private:


}; // class Component


class TestComponent : public Component
{
public:
	TestComponent() : a(0) {}
	TestComponent(int i) : a(i) {}
	int a;
};

template <typename T>
class ComponentArray
{
public:
	scl::varray<T>		m_components;
	scl::varray<Entity>	m_entities;
	

	void add(Entity e, const T& c)
	{
		m_components.push_back(c);
		m_entities.push_back(e);
	}
};

} // namespace cat


