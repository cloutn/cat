#pragma once

#include <scl/tree.h>
#include <scl/hash_table.h>
#include <scl/thread.h>

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
	// 同步策略（方案 A）：
	//   - alloc_id 用 scl::atomic_inc 做无锁递增，避免与 add/del/get 抢同一把锁
	//   - add/del/get 共用 m_mapMutex 保护 m_map（hash_table 的写非原子）
	//   - 当前 cat 业务侧 new/delete Object 都在主线程，物理 worker 接入后只读 get；
	//     在 single-writer-many-readers 场景下，未竞争 mutex 开销 ~25ns，profile 不可见。
	scl::hash_table<int, T*>	m_map;
	volatile int				m_id;			//current allocated id, 用 scl::atomic_inc 自增
	scl::mutex					m_mapMutex;		//保护 m_map 的 add/del/get
};

template <typename T>
cat::ObjectIDMap<T>::ObjectIDMap() : m_id(0)
{
}

template <typename T>
T* cat::ObjectIDMap<T>::get(const int id)
{
	scl::mutex_lock lock(&m_mapMutex);
	if (!m_map.is_init())
		return NULL;

	int fi = m_map.find_index(id);
	if (-1 == fi)
		return NULL;
	return m_map.get_value(fi);
}

template <typename T>
void cat::ObjectIDMap<T>::del(T* obj)
{
	if (NULL == obj)
		return;
	scl::mutex_lock lock(&m_mapMutex);
	assert(m_map.is_init());

	const int fi = m_map.find_index(obj->id());
	if (-1 == fi)
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
	scl::mutex_lock lock(&m_mapMutex);
	assert(m_map.is_init());

	m_map.add(obj->id(), obj);
}

template <typename T>
int ObjectIDMap<T>::alloc_id()
{
	// 用 atomic_inc 做无锁递增；返回的是 +1 后的新值
	const int newId = scl::atomic_inc(&m_id);
	// 溢出后 INT_MAX + 1 在 atomic_inc 仍然原子，但回绕成 INT_MIN ≤ 0；
	// 一旦返回 ≤ 0，调用方按 invalid id 处理，add 也会拒绝 < 0 的 id
	if (newId <= 0)
	{
		assert(false);
		return -1;
	}
	return newId;
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



