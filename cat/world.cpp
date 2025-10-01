#include "cat/world.h"

namespace cat {

static constexpr int MAX_COMPONENT_ARRAY_TABLE_SIZE = 32 * 1024;

World::World() : m_entityCounter(1)
{
	m_componentArrayTable.init(MAX_COMPONENT_ARRAY_TABLE_SIZE);
}

//void World::addComponent(Entity e, const Component& c)
//{
//
//}


} // namespace cat


