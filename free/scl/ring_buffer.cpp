////////////////////////////////////////////////////////////
//	RingBuffer2.cpp
//  caolei 2010.05.27
////////////////////////////////////////////////////////////

#include <memory.h>

#include "scl/ring_buffer.h"
#include "scl/file.h"
#include "scl/assert.h"

#define SCL_RING_BUFFER_USE_MEMORY_BARRIER

#ifdef SCL_RING_BUFFER_USE_MEMORY_BARRIER
#include "scl/memory_barrier.h"

#ifdef SCL_APPLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#endif

namespace scl
{

ring_buffer::ring_buffer()
:	m_buffer	(NULL),
	m_head		(0),
	m_tail		(0),
	m_maxSize	(0)
{
}

ring_buffer::~ring_buffer()
{
	::free(m_buffer);
	m_buffer = NULL;
}

int ring_buffer::write(const void* pSrc, int length)
{
	if (NULL == m_buffer)
	{
		assert(0);
		return 0;
	}
	const byte* pByteSrc = static_cast<const byte*>(pSrc);
	const int freelen = free();
	if (freelen < length)
	{
		assertf(0, "free = %d, length = %d", freelen, length);
		return 0;
	}
	//free() < len��֤����m_head > m_tail������£�ֱ��ʹ��tailLen�ǰ�ȫ��
 	int tailLength = m_maxSize - m_tail;
 	if (tailLength >= length)
 	{
 		memcpy(m_buffer + m_tail,	pByteSrc,	length);
 	}
 	else
 	{
 		memcpy(m_buffer + m_tail,	pByteSrc,				tailLength);
 		memcpy(m_buffer,			pByteSrc + tailLength,	length - tailLength);
 	}

	//make sure the data has been write to the buffer
#ifdef SCL_RING_BUFFER_USE_MEMORY_BARRIER
	scl_memory_barrier();
#endif

	_moveTail(length);
	return length;
}

int ring_buffer::read(void* pOut, int length)
{
	int readLength = peek(pOut, length);

	//make sure the data has been write to the out buffer
#ifdef SCL_RING_BUFFER_USE_MEMORY_BARRIER
	scl_memory_barrier();
#endif

	_moveHead(readLength);
 	return readLength;
}

void ring_buffer::clear()
{
	//����buffer
	m_head		= 0;
	m_tail		= 0;
}

void ring_buffer::clear_safe()
{
	drop(used());
}

int ring_buffer::peek(void* pPeek, int length)
{
	if (length <= 0)
	{
		assert(0);
		return 0;
	}
	if (NULL == m_buffer)
	{
		assert(0);
		return 0;
	}
	byte* pOut = static_cast<byte*>(pPeek);
	const int usedLength = used();
 	if (usedLength <= 0)
 		return 0;
 
 	if (length > usedLength)
 	{
 		length = usedLength;
 	}
	//len <= used()��֤����m_head < m_tail������£�ֱ��ʹ��tailLen�ǰ�ȫ��
	int tailLength = m_maxSize - m_head;
	if (tailLength >= length)
	{
		memcpy(pOut, m_buffer + m_head, length);
	}
	else
	{	
		memcpy(pOut, m_buffer + m_head,  tailLength);
		memcpy(pOut + tailLength, m_buffer, length - tailLength);
	}
	return length;
}

void ring_buffer::alloc(int size)
{
	if (NULL != m_buffer)
	{
		assert(0);
		return;
	}
	//��ʼ��������
	m_buffer = (byte*)malloc(size);
	memset(m_buffer, 0, size);

	//��ʼ����Ա����
	m_maxSize	= size;
	m_head		= 0;
	m_tail		= 0;
}

void ring_buffer::_moveHead(int length)
{
	//���̻߳�����������ʹ��m_tail��Ȼ������������ȡֵ�Ĳ�һ��
	//�ڶ��߳��¿��ܳ��������龰��
	//IsWrappedʹ����m_tail�жϣ����Կ���IsWrapped������false��m_head <= m_tail
	//���ǵ�ִ�е�assertf((m_head + length) <= m_tailʱ�������߳���buffer��д�����ݣ�����ɻ��ƣ�
	//m_tail��ͷ���ƣ���ʱ���Ծ�ʧ���ˡ�
	//if (IsWrapped())
	//{
	//	assertf((m_head + length) <= (m_tail + m_maxSize), "m_head = %d, m_tail = %d, length = %d", m_head, m_tail, length);
	//}
	//else
	//{
	//	assertf((m_head + length) <= m_tail, "m_head = %d, m_tail = %d, length = %d", m_head, m_tail, length);
	//}
	int head = m_head;
	if (head + length >= m_maxSize)
	{
		head = (head + length) % m_maxSize;
	}
	else
	{
		head += length;
	}
	m_head = head;
	assert(used() >= 0);
}

void ring_buffer::_moveTail(int length)
{
	//�μ�_moveHead˵��
	//if (IsWrapped())
	//{
	//	assertf((m_tail + length) < m_head, "m_head = %d, m_tail = %d, length = %d", m_head, m_tail, length);
	//}
	//else
	//{
	//	assertf((m_tail + length) < (m_head + m_maxSize), "m_head = %d, m_tail = %d, length = %d", m_head, m_tail, length);
	//}
	int tail = m_tail;
	if (tail + length >= m_maxSize)
	{
		tail = (tail + length) % m_maxSize;
	}
	else
	{
		tail += length;
	}
	m_tail = tail;
	assert(used() >= 0);
}

int ring_buffer::used() volatile 
{ 
	return static_used(m_head, m_tail, m_maxSize);
	//const int currentHead = m_head;
	//const int currentTail = m_tail;
	//int usedLength = 0;
	//if (currentHead > currentTail) // is wrapped
	//{
	//	//m_head��bufferβ���ĳ��� + ͷ����m_tail�ĳ���
	//	usedLength = (m_maxSize - currentHead) + currentTail;
	//}
	//else
	//{
	//	usedLength = currentTail - currentHead;
	//}
	//return usedLength;
}

int ring_buffer::free() volatile 
{ 
	return static_free(m_head, m_tail, m_maxSize);
}

void ring_buffer::drop(int length)
{
	assert(length <= used());
	_moveHead(length);
}

bool ring_buffer::is_wrapped() volatile 
{
	return static_isWrapped(m_head, m_tail);
}

bool ring_buffer::is_free_wrapped() volatile 
{
	return static_isFreeWrapped(m_head, m_tail);
}

int ring_buffer::static_free(const int head, const int tail, const int maxSize)
{
	return maxSize - static_used(head, tail, maxSize) - 1;  //�����1����Ϊm_tail��ռλ������ռ�ռ䲻��������
}

int ring_buffer::static_used(const int head, const int tail, const int maxSize)
{
	return head > tail ? maxSize - head + tail : tail - head;
}

bool ring_buffer::static_isFreeWrapped(const int head, const int tail)
{
	return head <= tail && (tail != 0) && (head != 0);
}


} //namespace scl

