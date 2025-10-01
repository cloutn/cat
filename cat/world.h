#pragma once

#include "cat/def.h"

#include "scl/hash_table.h"


namespace cat {

class Component;

class World
{
public:
	scl::hash_table<uint64, void*> m_componentArrayTable;
	
	int m_entityCounter;

	World();

	//void init();
	Entity createEntity()
	{
		return m_entityCounter++;
	}


	
	template <typename T>
	void addComponent(Entity e, const T& c);

private:

}; // class World

template <typename T>
void cat::World::addComponent(Entity e, const T& c)
{
	const uint64		tid = scl::type_id<T>();
	const int			tableIndex = m_componentArrayTable.find_index(tid);
	ComponentArray<T>* comps = NULL;
	if (-1 == tableIndex)
	{
		comps = new ComponentArray<T>();
		m_componentArrayTable.add(tid, comps);
	}
	else
	{
		comps = reinterpret_cast<ComponentArray<T>*>(m_componentArrayTable.get_value(tableIndex));
	}
	comps->add(e, c);
}

} // namespace cat



