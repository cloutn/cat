////////////////////////////////////////////////////////////////////////////////
//	ring_queue
//  2010.05.27 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/assert.h"

#define SCL_RING_QUEUE_USE_MEMORY_BARRIER

#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
#include "scl/memory_barrier.h"


#ifdef SCL_APPLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#endif

namespace scl {

////////////////////////////////////////////////////////////////////////////////
//	class ring_queue
//	ע��MAX_COUNT������Ĵ�С������m_tailռһλ������ʵ��������С��MAX_COUNT - 1
////////////////////////////////////////////////////////////////////////////////
template<typename T, int MAX_COUNT = 32>
class ring_queue
{
public:
	ring_queue();
	~ring_queue();

	//������Щpush_back����������ͬһ���߳��е���
	void	push_back		(const T& elem);
	// push_back_fast is not thread safe! because after all push_back_fast, the return T& object may be modified both by current thread and other thread.
	T&		push_back_fast_unsafe();
	T&		push_back_begin	(const int offset = 0);
	void	push_back_end	(const int count = 1);

	//��������������������ͬһ���߳��е���
	void	pop_front		(T& elem);
	T&		peek_front		();
	void	drop			(const int count = 1);

	int		used			() const;
	bool	empty			() const { return used() <= 0; }
	int		free			() const;
	int		capacity		() const { return MAX_COUNT; }
	void	clear_unsafe	();

private:
	T		m_queue[MAX_COUNT];
	int		m_head;
	int		m_tail;
};

template<typename T, int MAX_COUNT>
ring_queue<T, MAX_COUNT>::ring_queue()
{
	//��ʼ����Ա����
	m_head		= 0;
	m_tail		= 0;
}

template<typename T, int MAX_COUNT>
ring_queue<T, MAX_COUNT>::~ring_queue()
{
}

template<typename T, int MAX_COUNT>
void ring_queue<T, MAX_COUNT>::push_back(const T& elem)
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

template<typename T, int MAX_COUNT>
T& ring_queue<T, MAX_COUNT>::push_back_fast_unsafe()
{
	T& r = push_back_begin();
	push_back_end();
	return r;
}


template<typename T, int MAX_COUNT>
T& ring_queue<T, MAX_COUNT>::push_back_begin(const int offset)
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

template<typename T, int MAX_COUNT>
void ring_queue<T, MAX_COUNT>::push_back_end(int count)
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


template<typename T, int MAX_COUNT>
void ring_queue<T, MAX_COUNT>::pop_front(T& elem)
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

template<typename T, int MAX_COUNT>
T& ring_queue<T, MAX_COUNT>::peek_front()
{
	if (used() <= 0)
	{
		assert(false);
		static T empty;
		return empty;
	}
	return m_queue[m_head];
}

template<typename T, int MAX_COUNT>
void ring_queue<T, MAX_COUNT>::drop(const int count)
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


template<typename T, int MAX_COUNT>
int scl::ring_queue<T, MAX_COUNT>::used() const
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

template<typename T, int MAX_COUNT>
int scl::ring_queue<T, MAX_COUNT>::free() const
{
	return MAX_COUNT - used() - 1; //��1����Ϊm_tailռ��һλ
}

template<typename T, int MAX_COUNT>
void scl::ring_queue<T, MAX_COUNT>::clear_unsafe() 
{
	while (used())
		drop();
}



} //namespace scl

#ifdef SCL_APPLE
#pragma GCC diagnostic pop
#endif


