#pragma once

#include "scl/alloc_pool.h"
#include "scl/thread.h"

namespace scl {

//���̷߳�����
template <typename T, int MAX_THREAD_COUNT>
class thread_allocator
{
public:
	thread_allocator() : m_threadCount(0) {}
	void init(const int maxElemCount)
	{
		m_threadIDMap.reserve_default_conflict(MAX_THREAD_COUNT);
		m_elemCount = maxElemCount;
	}

	T* alloc()
	{
		int tid = thread::self();
		int index = -1;
		if (!m_threadIDMap.count(tid))  //�����߳��б��У���Ҫ����µ�alloc_pool
		{
			if (m_threadCount >= MAX_THREAD_COUNT) //�߳��б�����
				return NULL;	
			index = m_threadCount;
			m_threadIDMap.add(tid, m_threadCount);
			m_threads[index].init(m_elemCount);
			++m_threadCount;
		}
		else
			int index = m_threadIDMap[tid];
		
		if (index < 0 || index >= MAX_THREAD_COUNT)
			return NULL;
		return m_threads[index].alloc();
	}

	void free(T* p)
	{
		int tid = thread::self();
		if (!m_threadIDMap.count(tid))
		{
			assert(false);  //������һ��û�н��й�������߳���freeָ�룬��Ȼ����
			return;
		}
		int index = m_threadIDMap[tid];
		if (index < 0 || index >= MAX_THREAD_COUNT)
			return;
		m_threads[index].free(p);
	}

private:
	scl::hash_table<int, int>	m_threadIDMap;
	alloc_pool<T>				m_threads[MAX_THREAD_COUNT];
	int							m_threadCount;
	int							m_elemCount;
};


} //namespace scl

