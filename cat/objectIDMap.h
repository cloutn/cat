#pragma once

#include <scl/tree.h>
#include <scl/hash_table.h>

namespace cat {

template <typename T>
class ObjectIDMap
{
public:
	ObjectIDMap();

	void	init		(const int maxCount);
	int		alloc_id	();
	void	add			(T* obj);
	void	del			(T* obj);
	T*		get			(const int id);

private:
	scl::hash_table<int, T*>	m_map;
	int							m_id;			//current allocated id
};

template <typename T>
cat::ObjectIDMap<T>::ObjectIDMap() : m_id(0)
{
}

template <typename T>
T* cat::ObjectIDMap<T>::get(const int id)
{
	if (!m_map.is_init())
		return NULL;

	int fi = m_map.find_index(id);
	if (fi == -1)
		return NULL;
	return m_map.get_value(fi);
}

template <typename T>
void cat::ObjectIDMap<T>::del(T* obj)
{
	if (NULL == obj)
		return;
	assert(m_map.is_init());

	const int fi = m_map.find_index(obj->id());
	if (fi == -1)
		return;
	m_map.erase(obj->id());
}

template <typename T>
void ObjectIDMap<T>::add(T* obj)
{
	if (NULL == obj)
		return;
	assert(m_map.is_init());

	m_map.add(obj->id(), obj);
}

template <typename T>
int ObjectIDMap<T>::alloc_id()
{
	assert(m_id + 1 > 0);
	return ++m_id;
}

template <typename T>
void ObjectIDMap<T>::init(const int maxCount)
{
	if (m_map.is_init())
		return;
	const int MAX_CONFLICT = 8;
	m_map.init(MAX_CONFLICT * maxCount);
}

//template <typename PT> class remove_ptr { public: typedef PT type; };
//template <typename PT> class remove_ptr<PT*> { public: typedef PT type; };

} //namespace cat



