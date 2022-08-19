////////////////////////////////////////////////////////////////////////////////
//	vring_queue
//  2015.04.08 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/assert.h"

#define SCL_RING_QUEUE_USE_MEMORY_BARRIER

#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
#include "scl/memory_barrier.h"
#endif

namespace scl {

////////////////////////////////////////////////////////////////////////////////
//	class vring_queue
//	ע��maxCount������m_tailռһλ������ʵ��������С��maxCount-1
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class vring_queue
{
public:
	vring_queue();
	~vring_queue();

	void alloc(const int maxCount)
	{
		assert(NULL == m_queue);
		m_queue = new T[maxCount];
		MAX_COUNT = maxCount;
	}

	//������Щpush_back����������ͬһ���߳��е���
	void	push_back		(const T& elem);
	T&		push_back_fast	();
	T&		push_back_begin	(const int offset = 0);
	void	push_back_end	(const int count = 1);

	//��������������������ͬһ���߳��е���
	void	pop_front		(T& elem);
	T&		peek_front		();
	void	drop			(const int count = 1);

	int		used			() const;
	int		free			() const;
	int		capacity		() const { return MAX_COUNT; }

private:
	T*		m_queue;
	int		m_head;
	int		m_tail;
	int		MAX_COUNT;
};

template<typename T>
vring_queue<T>::vring_queue()
{
	//��ʼ����Ա����
	m_queue		= NULL;
	m_head		= 0;
	m_tail		= 0;
	MAX_COUNT	= 0;
}

template<typename T>
vring_queue<T>::~vring_queue()
{
	if (NULL != m_queue)
	{
		delete[] m_queue;
		m_queue = NULL;
	}
}

template<typename T>
void vring_queue<T>::push_back(const T& elem)
{
	if (free() <= 0)
	{
		assert(false);
		return;
	}
	int tail = m_tail;

	m_queue[tail] = elem;

	//ע���m_tail���޸ı�����ԭ�ӵģ��������ܱ�֤�̰߳�ȫ
	tail++;
	if (tail >= MAX_COUNT)
	{
		tail = tail % MAX_COUNT;
	}

#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
	scl_memory_barrier();
#endif

	m_tail = tail;
}

template<typename T>
T& vring_queue<T>::push_back_fast()
{
	T& r = push_back_begin();
	push_back_end();
	return r;
}


template<typename T>
T& vring_queue<T>::push_back_begin(const int offset)
{
	if (free() <= offset)
	{
		assert(false);
		throw 1;
	}
	int tail = m_tail + offset;
	if (tail >= MAX_COUNT)
		tail = tail % MAX_COUNT;
	return m_queue[tail];
}

template<typename T>
void vring_queue<T>::push_back_end(int count)
{
	//ע���m_tail���޸ı�����ԭ�ӵģ��������ܱ�֤�̰߳�ȫ
	int tail = m_tail;
	tail += count;
	if (tail >= MAX_COUNT)
	{
		tail = tail % MAX_COUNT;
	}
#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
	scl_memory_barrier();
#endif

	m_tail = tail;
}


template<typename T>
void vring_queue<T>::pop_front(T& elem)
{
	if (used() <= 0)
	{
		assert(false);
		return;
	}
	int head = m_head;

	elem = m_queue[head];

	//ע��,ֻ�����������m_head�����޸ģ�
	//����m_head���޸ı���һ���������ֵ�������ܳ����м�ֵ��
	//����ᱻ�����̶߳�ȡ���м�ֵ
	head++;
	if (head >= MAX_COUNT)
	{
		head = head % MAX_COUNT;
	}
#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
	scl_memory_barrier();
#endif

	m_head = head;
}

template<typename T>
T& vring_queue<T>::peek_front()
{
	if (used() <= 0)
	{
		assert(false);
		static T empty;
		return empty;
	}
	return m_queue[m_head];
}

template<typename T>
void vring_queue<T>::drop(const int count)
{
	if (used() <= 0)
	{
		assert(false);
		return;
	}
	int head = m_head;

	//ע��,ֻ�����������m_head�����޸ģ�
	//����m_head���޸ı���һ���������ֵ�������ܳ����м�ֵ��
	//����ᱻ�����̶߳�ȡ���м�ֵ
	head += count;
	if (head >= MAX_COUNT)
	{
		head = head % MAX_COUNT;
	}
#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
	scl_memory_barrier();
#endif

	m_head = head;
}


template<typename T>
int scl::vring_queue<T>::used() const
{
	const int currentHead = m_head;
	const int currentTail = m_tail;
	int usedLength = 0;
	if (currentHead > currentTail) // is wrapped
	{
		//m_head��bufferβ���ĳ��� + ͷ����m_tail�ĳ���
		usedLength = (MAX_COUNT - currentHead) + currentTail;
	}
	else
	{
		usedLength = currentTail - currentHead;
	}
	return usedLength;
}

template<typename T>
int scl::vring_queue<T>::free() const
{
	return MAX_COUNT - used() - 1; //��1����Ϊm_tailռ��һλ
}



} //namespace scl
