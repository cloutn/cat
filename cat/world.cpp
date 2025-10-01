
#include "cat/world.h"
#include "cat/component.h"

namespace cat {

static constexpr int MAX_COMPONENT_ARRAY_TABLE_SIZE = 32 * 1024;

World::World() : m_entityCounter(1)
{
	m_componentArrayTable.init(MAX_COMPONENT_ARRAY_TABLE_SIZE);
}

World::~World()
{
	scl::varray<ComponentArrayBase*> arrays;
	m_componentArrayTable.get_values(arrays);
	for (int i = 0; i < arrays.size(); ++i)
	{
		delete arrays[i];
	}
}

//void World::addComponent(Entity e, const Component& c)
//{
//
//}


} // namespace cat


