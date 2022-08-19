#pragma once
////////////////////////////////////////////////////////////////////////////////
//	alloc_pool	
//	ʹ�ó��������ڴ�
////////////////////////////////////////////////////////////////////////////////

#include "scl/stack.h"
#include <new>	//placement new
#include <stdlib.h> //malloc()
#include <memory.h> //memset

template<typename T>
class alloc_pool
{
public:
	alloc_pool() : m_memory(NULL), m_flags(NULL), m_allocCount(0), m_hasInit(false), m_maxSize(0) {}

	inline void			init			(int maxSize);
	inline void			release			();
	bool				is_full			() const { return m_allocCount == 0 && m_free.size() == 0; }
	bool				has_init		() const { return m_hasInit; }
	int					free_size		() const { return m_allocCount + m_free.size(); }
	//void				set_thread_id	(const int threadID) { m_threadID = id; }
	//int					thread_id		() const { return m_threadID; }
	bool				in_pool			(T* elem) const;

	inline T*			alloc			();
	inline void			free			(T* const pElem);

private:
	T*					m_memory;
	char*				m_flags;
	scl::vstack<int>	m_free;
	int					m_allocCount;
	bool				m_hasInit;
	int					m_maxSize; //����m_free.capacity() ����Ч�ʸ���
	//int					m_threadID;
};

template<typename T>
inline void alloc_pool<T>::init(const int maxSize)
{
	assert(!m_hasInit);
	assert(maxSize > 0);

	//Ϊ�˹��initʱ��T�Ĺ��캯���ĵ��ã�����ʹ��malloc
	//����Ϊ�˷���new_tracer�ܹ���¼alloc_pool������ڴ棬�����ָĳ���new byte[]
	m_memory = static_cast<T*>(static_cast<void*>(new byte[maxSize * sizeof(T)]));
	if (NULL == m_memory)
	{
		assert(0);
		return;
	}
	m_free.reserve(maxSize);
	m_allocCount = maxSize; 
	m_flags = new char[maxSize];
	memset(m_flags, 0, sizeof(m_flags[0]) * maxSize);
	m_hasInit = true;
	m_maxSize = maxSize;
}

template<typename T>
inline void alloc_pool<T>::release()
{
	//����ڴ�й©
	//m_free.size()�Ǳ��ͷŵ�����Ԫ���ܺ�
	//m_allocCount�����m_memory�л�δ�������ȥ������
	//���������ܺ���release��ʱ�����ͷ���������������ͬ�����ܱ�֤����alloc���ڴ涼��free��
	assert(m_free.size() + m_allocCount == m_free.capacity());

	m_hasInit = false;
	delete[] (byte*)m_memory;

	delete[] m_flags;
	//::free(m_memory);
}

template<typename T>
inline bool alloc_pool<T>::in_pool(T* elem) const
{
	return elem >= m_memory && elem < m_memory + m_maxSize; 
}

template<typename T>
inline T* alloc_pool<T>::alloc()
{
	if (!m_hasInit)
	{
		assert(0);
		return NULL;
	}
	if (m_allocCount > 0)
	{
		--m_allocCount;
		T* p = new (&m_memory[m_allocCount]) T;
		assert(m_flags[m_allocCount] == 0);
		m_flags[m_allocCount] = 1;
		return p;
	}
	else if (m_free.size() > 0)
	{
		int index = m_free.pop();
 		T* p = new (&m_memory[index]) T;
		assert(m_flags[index] == 0);
		m_flags[index] = 1;
 		return p;
	}
	else
	{
		//throw(1);	//TODO �����Ӧ�쳣
		assert(0);
		return NULL;
	}
}

template<typename T>
inline void alloc_pool<T>::free(T* const pElem)
{
	if (!m_hasInit)
	{
		assert(0);
		return;
	}
	if (NULL != pElem)
	{	
		uint64 index = pElem - &(m_memory[0]); //TODO ����̫�����ˣ�Ӧ�ö���һ��������ULONG_PTR�Ķ���
		int32 index32 = static_cast<int32>(index);

		//���ָ���Ƿ��ڷ�Χ�ڣ�������ڷ������ڲ��������������̷߳�����ڴ棬��Ҫ����ϲ���߳��߼�
		if (index32 < 0 || index32 >= m_free.capacity())
		{
			assertf(false, "index = %d", index32);
			return;
		}

		//����
		pElem->~T();
		
		//��flag
		int i32 = static_cast<int>(index);
		assertf(m_flags[i32] == 1, "flag[%d] = %d", i32, m_flags[i32]);
		m_flags[i32] = 0;

		//��ӵ����ö�ջ��
		m_free.push(i32);
	}
}
