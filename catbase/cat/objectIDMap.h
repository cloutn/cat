#pragma once

#include <scl/tree.h>
#include <scl/hash_table.h>

#include <limits.h>

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
	// alloc_id 溢出后会返回 -1，这里挡住，避免 -1 sentinel 被当成正常 key 入表
	if (obj->id() < 0)
		return;
	assert(m_map.is_init());

	m_map.add(obj->id(), obj);
}

template <typename T>
int ObjectIDMap<T>::alloc_id()
{
	// 不要写 m_id + 1 > 0：signed int 溢出是 UB，编译器有权把它优化掉。
	// release 也必须能拦下，避免回绕成负数后污染 -1 这种 sentinel。
	if (m_id >= INT_MAX - 1)
	{
		assert(false);
		return -1;
	}
	return ++m_id;
}

template <typename T>
void ObjectIDMap<T>::init(const int maxCount)
{
	if (m_map.is_init())
		return;
	const int MAX_CONFLICT = 8;
	// 反向写避免乘法本身 UB：maxCount > INT_MAX / MAX_CONFLICT 即一定溢出
	if (maxCount <= 0 || maxCount > INT_MAX / MAX_CONFLICT)
	{
		assert(false);
		return;
	}
	m_map.init(MAX_CONFLICT * maxCount);
}

//template <typename PT> class remove_ptr { public: typedef PT type; };
//template <typename PT> class remove_ptr<PT*> { public: typedef PT type; };

} //namespace cat



