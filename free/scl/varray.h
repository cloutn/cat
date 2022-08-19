////////////////////////////////////////////////////////////////////////////////
//	array and varray
//	2010.04.24 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/assert.h"
#include "scl/math.h"
#include <new>	//placement new
		
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#include <stdlib.h>	
#include <string.h>	
#endif

#include <algorithm>

namespace scl{

////////////////////////////////////////////////////////////////////////////////
//	class varray
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class varray
{
public:
	varray(): m_array(NULL), m_size(0), m_maxSize(0), m_option(0) {}
	~varray();

	//stl���ݽӿڡ�iterator��ʱ��ΪT*
	typedef T			value_type;
	typedef int			size_type;
	void	push_back	(const T& elem);
	void	erase		(const int index);
	void	clear		();
	int		size		() const 				{ return m_size; };
	int		max_size	() const 				{ return m_maxSize; }
	T*		begin		()						{ return m_array; }
	T*		end			()						{ return &(m_array[m_size]); }
	const T* begin		() const				{ return m_array; }
	const T* end		() const				{ return &(m_array[m_size]); }
	T*		rbegin		()						{ return &(m_array[m_size - 1]); }
	T*		rend		()						{ return &(m_array[-1]); }
	void	resize		(const int size);		
	void	reserve		(const int capacity)	{ _grow_to(capacity); }
	bool	empty		() const				{ return m_size == 0; }
	int		capacity	() const		{ return m_maxSize; }
	void	assign		(const T* other, const int count, const int start = 0);
	void	reverse		() { for (int i = 0; i < m_size / 2; ++i) swapmem2(m_array[i], m_array[m_size - 1 - i]); }
	template <class _Pr>
	inline void sort(_Pr _Pred) { std::sort(m_array, m_array + m_size, _Pred);	}
	inline void sort()			{ std::sort(m_array, m_array + m_size);			}

	inline void qsort(int (*comp_func)(const void*, const void*)) { return ::qsort(m_array, m_size, sizeof(T), comp_func); }

	template <class _Pr>
	int binary_search2	(const T& elem, _Pr compare)	const	{ return scl::binary_search<T, _Pr>(m_array, 0, size() - 1, elem, compare); }
	int binary_search	(const T& elem)					const	{ return scl::binary_search<T>(m_array, 0, size() - 1, elem); }

	//template <class SearchT, class _Pr>
	//int binary_search_type(const SearchT& elem, _Pr customCompare);

	//stl��û����Ӧ�Ĺ��ܽӿ�
	//T&		push_back_fast		();		//��T�д���ǳ����ָ��ʱ��������Ҫ���ⲻ��Ҫ����ʱ���󿽱�ʱ��ʹ��push_back_fast�ӿ�
	T&		push_back_fast		(const int count = 1);		//��T�д���ǳ����ָ��ʱ��������Ҫ���ⲻ��Ҫ����ʱ���󿽱�ʱ��ʹ��push_back_fast�ӿ�
	void	erase_fast			(const int index);
	void	erase_element		(const T& elem);
	void	erase_element_fast	(const T& elem);	//����������Ԫ�ص�˳�򲻹��ĵ�ʱ��ʹ��RemoveFast�ӿ����Ч��
	int		find				(const T& elem) const;
	bool	contains			(const T& elem) const { return find(elem) != -1; }
	void	zero_memory			(const int start = 0, const int len = -1);
	void	disable_constructor	() { m_option |= OPTION_DISABLE_CONSTRUCTOR; }
	void	disable_destructor	() { m_option |= OPTION_DISABLE_DESTRUCTOR; }
	void	insert				(const int index, const T& elem);
	
	const T& operator[](int index) const { if (index >= m_maxSize || index < 0) { assert(0); throw 1; } return m_array[index]; }
	T&		 operator[](int index)		 { if (index >= m_maxSize || index < 0) { assert(0); throw 1; } return m_array[index]; }
	T*		 c_array			()			{ return m_array; }
	const T* c_array			() const	{ return m_array; }

	template <typename StreamerT>
	void map(StreamerT& s)
	{
		s << m_size;
		for (int i = 0; i < size(); ++i)
			s << m_array[i];
	}

	enum OPTION
	{
		OPTION_DISABLE_CONSTRUCTOR	= 0x01, //���ù��캯��
		OPTION_DISABLE_DESTRUCTOR	= 0x02, //������������

	};

private:
	void _grow_to	(const int count);		//������ָ����С�������󣬺�_grow����ÿ��������STEP��ĳ��������ͬ��_grow_to��ʹ�������capacity�ϸ�Ϊ����Ĳ���count
	void _grow		(const int count = -1); //�Զ������������ָ��count����������Ϊ����ǰSTEP�������ָ��count��������϶���������count������capacity��һ����count
	bool _disable_constructor	() const { return (m_option & OPTION_DISABLE_CONSTRUCTOR) != 0; } //�Ƿ���ù��캯��
	bool _disable_destructor	() const { return (m_option & OPTION_DISABLE_DESTRUCTOR	) != 0; } //�Ƿ������������

protected:
	T*		m_array;
	int 	m_size;
	int 	m_maxSize;
	char	m_option;
};

template<typename T>
inline void swapmem2(T& a, T& b)
{
	if (&a == &b)
		return;
	//if (a == b)  //operator==ֻ�ṩ���߼��ϵ�������ж�
	//	return;
	int* pIntA = reinterpret_cast<int*>(&a);
	int* pIntB = reinterpret_cast<int*>(&b);
	for (uint i = 0; i < sizeof(T) / sizeof(int); ++i)
	{
		int t = *pIntA;
		*pIntA = *pIntB;
		*pIntB = t;
		++pIntA;
		++pIntB;
	}

	byte* pByteA = reinterpret_cast<byte*>(pIntA);
	byte* pByteB = reinterpret_cast<byte*>(pIntB);
	for (uint j = 0; j < sizeof(T) % sizeof(int); ++j)
	{
		byte t = *pByteA;
		*pByteA = *pByteB;
		*pByteB = t;
		++pByteA;
		++pByteB;
	}
}


////////////////////////////////////////////////////////////////////////////////
//	varrayʵ��
////////////////////////////////////////////////////////////////////////////////
template<typename T> 
varray<T>::~varray()
{ 
	//������varray��������ʱ�򣬲�Ӧ���ٿ���_disable_destructorѡ��������ɴ����array�ڵ�element����������û�б����õ����
//	if (!_disable_destructor())
	{
		for (int i = 0; i < size(); ++i)
			m_array[i].~T(); //!!!ע�⣬���Ҫʹ�����ܵ��õ�class T��������������ôvarray���������������class T���������壬��������Ϊ��Ա����ֻ��һ��T*ָ�룬�ͽ���ʹ��һ��T��ǰ������
	}
	::free(m_array);
	m_array		= NULL;
	m_maxSize	= 0;
	m_size		= 0;
}

template<typename T> 
void varray<T>::push_back(const T& elem)
{
	if (m_size >= m_maxSize)
	{
		_grow();
	}

	if (!_disable_constructor())
		::new (&m_array[m_size]) T;

	m_array[m_size] = elem;
	++m_size;
}

//template<typename T> 
//T& varray<T>::push_back_fast()
//{
//	if (m_size >= m_maxSize)
//	{
//		_grow();
//	}
//	T* p = NULL;
//	if (!_disable_constructor())
//		p = ::new (&m_array[m_size]) T;
//	else
//		p = &m_array[m_size];
//
//	++m_size;
//	if (NULL == p)
//	{
//		assert(0);
//		throw 1;
//	}
//	return *p;
//}
//

template<typename T> 
T& varray<T>::push_back_fast(const int count)
{
	if (m_size + count > m_maxSize)
	{
		_grow(m_size + count);
	}
	T* p = &m_array[m_size];
	if (!_disable_constructor())
	{
		for (int i = 0; i < count; ++i)
			::new (&m_array[m_size + i]) T;
	}

	m_size += count;
	if (NULL == p)
	{
		assert(0);
		throw 1;
	}
	return *p;
}


template<typename T> 
void varray<T>::erase_element(const T& elem)
{
	int delIdx = find(elem);
	if (-1 == delIdx)
		return;

	erase(delIdx);
}

template<typename T> 
void varray<T>::erase(const int index)
{
	if (index < 0 || index >= m_size)
		return;

	//������������
	if (!_disable_destructor())
		m_array[index].~T();

	if (index < m_size - 1)
		memmove(&m_array[index], &m_array[index + 1], sizeof(T) * (m_size - index));

	--m_size;
}

template<typename T> 
void varray<T>::clear()
{
	//������������
	if (!_disable_destructor())
	{
		for (int i = 0; i < m_size; ++i)
			m_array[i].~T();
	}
	m_size = 0;
}

template<typename T>
void varray<T>::erase_element_fast(const T& elem)
{
	int delIdx = find(elem);
	if (-1 == delIdx)
		return;

	erase_fast(delIdx);
}

template<typename T>
void varray<T>::erase_fast(const int index)
{
	if (index < 0 || index >= m_size)
	{
		assert(0);
		return;
	}

	//������������
	if (!_disable_destructor())
		m_array[index].~T();

	//����Ҫ������������T�а���ָ���ʱ��
	//�ᵼ��m_array[m_count - 1]��������deleteͬһ��ָ�����Σ�m_array[index]�л���һ����ͬ��ָ�룩
	//ͬʱ��������һ���µ�T temp���������м��������Ϊ�����ᵼ��temp������
	//ʹ��ͬ����ͬһ��ָ�뱻����delete�����⣨temp�ڵ�ָ��ͱ�temp�������elem�ڵ�ָ�룩
	//2013.10.15 ��T������Ϊָ��ʱ��swapmem2�ڶ��߳��²���ȫ��ֱ�ӽ�����ȫ�������ᵽ�����⣬����T����Ŀ������캯����������Ǳ����������
	//m_array[index] = m_array[m_count - 1];
	if (index != m_size - 1)
	{
		//m_array[index] = m_array[m_size - 1];
		swapmem2(m_array[index], m_array[m_size - 1]);
	}
	m_size--;
}

template<typename T>
int varray<T>::find(const T& elem) const
{
	int foundIdx = -1;
	for (int i = 0; i < m_size; ++i)
	{
		if (m_array[i] == elem)
		{
			foundIdx = i;
			break;
		}
	}
	return foundIdx;
}


template<typename T>
void varray<T>::assign(const T* other, const int count, const int start)
{
	if (count == 0)
		return;
	if (start + count > m_maxSize)
	{
		_grow(start + count);	//����֮���Ե���_grow������_grow_to������Ϊassign�����push_back��Ԫ�أ��ᵼ���ٴ�_grow
	}

	//����ڴ��Ƿ��ص�
	assert(m_array != other);
	if (m_array < other)
		assert(m_array + start + count < other);
	if (other < m_array)
		assert(other + count < m_array);

	memcpy(m_array + start, other, count * sizeof(T));
	m_size = start + count;
}

template<typename T>
void varray<T>::_grow(const int count)
{
#ifdef SCL_DEBUG
	static int grow_c = 0;
	static int total_mem = 0;
	grow_c++;
#endif

	const int STEP = 8;
	//const int oldMaxSize = m_maxSize;

	//ȷ��Ҫ�������Ĵ�С
	int target = count;
	if (target == -1)
		target = m_maxSize + 1;
	assert(target > 0 && target > m_maxSize);
	if (m_maxSize == 0)
		m_maxSize = 16;
	while (m_maxSize < target)
		m_maxSize *= STEP;

	//�����¿ռ�
	T* p = static_cast<T*>(::malloc(sizeof(T) * m_maxSize)); //new T[m_maxSize];
	if (NULL == p)
		return;
	if (_disable_constructor()) //�����˹��캯������������Ҫ���´������ڴ����򵥵���չ���	//TODO �Ƿ�������ѡ�������һ����?
		memset((void*)(void*)(void*)p, 0, sizeof(T) * m_maxSize);
	
#ifdef SCL_DEBUG
	total_mem += m_maxSize * sizeof(T);
#endif

	//���������ݵ��¿ռ�
	if (m_array != NULL)
	{
		if (m_size > 0)
		{
			assert(m_size <= m_maxSize);
			memcpy((void*)p, (void*)m_array, sizeof(T) * m_size);
		}
		::free(m_array);
#ifdef SCL_DEBUG
		total_mem -= m_size * sizeof(T);
		printf("_grow = %d, count = %d, total mem = %dk, sizeof(T) = %d\n", m_maxSize, grow_c, total_mem / 1024, sizeof(T));
#endif
	}
	m_array = p;
}

//������ָ����С��������array��capacity�ϸ�Ϊcount
template <typename T>
void varray<T>::_grow_to(const int count)
{
	if (m_maxSize >= count)
		return;

	m_maxSize = count;

	//�����¿ռ�
	T* p = static_cast<T*>(::malloc(sizeof(T) * m_maxSize)); //new T[m_maxSize];
	if (NULL == p)
		return;
	if (_disable_constructor()) //�����˹��캯������������Ҫ���´������ڴ����򵥵���չ���	//TODO �Ƿ�������ѡ�������һ����?
		memset((void*)(void*)(void*)p, 0, sizeof(T) * m_maxSize);

	//���������ݵ��¿ռ�
	if (m_array != NULL)
	{
		if (m_size > 0)
			memcpy((void*)p, (void*)m_array, sizeof(T)* m_size);
		::free(m_array);
	}
	m_array = p;
}
template <typename T>
void varray<T>::resize(const int size)
{
	if (size > m_maxSize)
	{
		assert(false);
		return;
	}
	if (size > m_size)
	for (int i = m_size; i < size; ++i)
		push_back_fast();

	if (size < m_size)
	for (int i = m_size - 1; i >= size; --i)
		erase(i);
};

template <typename T>
void varray<T>::zero_memory(const int start, const int len) 
{ 
	int l = len >= 0 ? len : capacity();
	if (l > 0)
		memset(m_array + start, 0, sizeof(T) * l); 
}

template <typename T>
void varray<T>::insert(const int index, const T& elem)
{
	if (m_size >= m_maxSize)
		_grow();

	if (index > m_size || index < 0)
		return;
	if (index < m_size)
		memmove(&m_array[index + 1], &m_array[index], sizeof(T) * (m_size - index));
	m_array[index] = elem;
	++m_size;
}
//
//template <typename T>
//template <class _Pr>
//int varray<T>::binary_search(const T& elem, _Pr compare)
//{
//	
//	//int		left	= 0;
//	//int		right	= size() - 1;
//	//int		mid		= -1;
//	//while (left <= right)
//	//{
//	//	mid = (left + right) / 2; // mid = left + (right - left) / 2 ==>  mid = (left + right) / 2
//	//	T& cur = m_array[mid];
//	//	if (compare(cur, elem))
//	//		right = mid - 1;
//	//	else if (compare(elem, cur))
//	//		left = mid + 1;
//	//	else
//	//		break;
//	//}
//	//return mid;
//}

} //namespace scl
