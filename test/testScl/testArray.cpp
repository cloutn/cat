
#include "testArray.h"

#include "scl/array.h"
#include "scl/varray.h"
#include "scl/string.h"
#include "scl/wstring.h"
#include "scl/assert.h"
#include "scl/page_array.h"

#include <stdio.h>

using scl::wstring;
using scl::array;
using scl::varray;
using scl::parray;
using scl::parray_const;
using scl::swapmem;
//
//template<typename T>
// void printArray(parray<T>& a)
//{
//	for (int i = 0; i < a.size(); ++i)
//	{
//		printf("%d\r\n", a[i]);
//	}
//}

class ShallowCopyObject
{
public:
	ShallowCopyObject()
	{
		m_pShallowPointer = new int[5];
	}
	~ShallowCopyObject()
	{
		delete[] m_pShallowPointer;
		m_pShallowPointer = NULL;
	}
	//void operator=(ShallowCopyObject& other)
	//{
	//	if (this == &other)
	//		return;
	//	if (NULL != this->m_pShallowPointer)
	//	{
	//		delete[] this->m_pShallowPointer;
	//		this->m_pShallowPointer = NULL;
	//	}
	//	this->m_pShallowPointer = other.m_pShallowPointer;
	//	other.m_pShallowPointer = NULL;
	//	memcpy(this->m_message, other.m_message, sizeof(this->m_message));
	//	memcpy(this->m_short,	other.m_short, sizeof(this->m_short));
	//}
	int*	m_pShallowPointer;
	wchar	m_message[10];
	short	m_short[64];
};

void testArrayShallowCopy()
{
	//ShallowCopyObject rawArray[10];
	const int COUNT = 6;
	array<ShallowCopyObject, COUNT> a;
	for (int i = 0; i < COUNT; ++i)
	{
		ShallowCopyObject& o = a.push_back_fast();
		int* p = o.m_pShallowPointer;
		p = NULL;
	}

	//这里remove会导致ShallowCopyObject赋值操作符被调用
	a.erase_fast(2);
	a.erase_fast(4);
	a.erase(3);
//	assert(a.size() == 8);
}

struct SwapMemoryData
{
	int16 	a1;
	int	  	a2;
	wstring<17> m;
	float 	a3;
	uint16	a4;
	double	a5;
};

void testSwapMemory()
{
	const float q[4] = { 5.676f, 4.565f, 2.3323f, 1.22313f };

	SwapMemoryData a, b;
	a.a1 = -3;
	a.a2 = 9282;
	a.m = L"a我z们";
	a.a3 = q[0];
	a.a4 = 1984;
	a.a5 = q[1];
	double z1 = a.a5;

	b.a1 = 65;
	b.a2 = 5672;
	b.m	 = L"r库rr卡";
	b.a3 = q[2];
	b.a4 = 1876;
	b.a5 = q[3];
	double z2 = b.a5;

	swapmem(a, b);

	assert(b.a1 == -3);
	assert(b.a2 == 9282);
	assert(b.m == L"a我z们");
	assert(b.a3 == q[0]);
	assert(b.a4 == 1984);
	assert(b.a5 == z1);

	assert(a.a1 == 65);
	assert(a.a2 == 5672);
	assert(a.m == L"r库rr卡");
	assert(a.a3 == q[2]);
	assert(a.a4 == 1876);
	assert(a.a5 == z2);

	wstring<17> s1, s2;
	s1 = L"aaa";
	s2 = L"bbb";
	swapmem(s1, s2);
	assert(s1 == L"bbb");
	assert(s2 == L"aaa");
}

class VArrayObject
{
public:
	int *ptr;

	VArrayObject() 
	{ 
		static int c = 0;
		c++;
		ptr = new int[3]; 
		//printf("c = %d, %x\n", c, ptr);
	}
	~VArrayObject() 
	{ 
		static int c2 = 0;
		c2++;
		//printf("~~~c = %d, %x\n", c2, ptr);
		delete[] ptr; ptr = NULL; 
	}
};


class VArray_Point
{
public:
	int x;
	int y;
	VArray_Point()	 { x = 0; y = 0; }
	VArray_Point(int _x, int _y) { x = _x; y = _y; }
};


class VArray_Wrapper
{
public:
	varray<VArray_Point>	points;
	bool					containsEnd;
	~VArray_Wrapper() 
	{
	}
	VArray_Wrapper() : containsEnd(false) 
	{
	}
};

void testVArray2()
{
	varray<VArray_Wrapper> wrappers;
	wrappers.push_back_fast();
	for (int i = 0; i < 18; ++i)
	{
		VArray_Wrapper& wrapper			= wrappers.push_back_fast();
		varray<VArray_Point>& oldPoints		= wrappers[0].points;
		wrapper.points.assign(oldPoints.c_array(), oldPoints.size());
		wrapper.points.push_back(VArray_Point(i + 1, i + 1));
		wrappers.erase_fast(0);
	}
}

void testVArray()
{
	varray<int> v;

	for (int i = 0; i < 16; ++i)
		v.push_back(i);
	assert(v.capacity() == 16);

	v.push_back(32);
	assert(v.capacity() == 128);

	const int OTHER_SIZE = 2048;
	int other[OTHER_SIZE];
	for (int i = 0; i < OTHER_SIZE; ++i)
		other[i] = i;
	v.assign(other, OTHER_SIZE);
	assert(v.size() == OTHER_SIZE);
	assert(v.capacity() == 8192);
	assert(v[1024] == 1024);


	varray<VArrayObject> v2;
	assert(v2.capacity() == 0);
	for (int i = 0; i < 16; ++i)
	{
		VArrayObject& o = v2.push_back_fast();
		assert(v2.capacity() == 16);
	}

	
	v2.push_back_fast();
	v2.erase_fast(2);
	v2.erase_fast(4); //看看是否正确delete

	v2.push_back_fast();
	v2.push_back_fast();
	v2.push_back_fast();

	assert(v2.capacity() == 128);
}

void testPArray()
{
	array<int, 16> a1;
	a1.push_back(1);
	a1.push_back(2);
	parray_const<int> pa1(a1);
	assert(a1.size() == pa1.size());
	for (int i = 0; i < pa1.size(); ++i) 
		assert(a1[i] == pa1[i]);
}

void testPageArray_check(const scl::page_array<int>& a)
{
	for (int i = 0; i < 100; ++i)
		assert(a[i] == i);
}

void testPageArray()
{
	scl::page_array<int> a;
	a.reserve(10, 10);
	for (int i = 0; i < 100; ++i)
		a.push_back(i);

	testPageArray_check(a);	
	//a.push_back(100);
}

 void testArray()
{
	array<int, 10> a;

	//测试add函数
	for (int i = 0; i < 10; ++i)
	{
		a.push_back(i + 1);
		assert(i + 1 == a[i]);
	}

	//测试remove函数
	a.erase_element(6);

	assert(a.size() == 9);
	assert(a[5] == 7);
	assert(a[8] == 10);

	a.erase_element_fast(3);
	assert(a.size() == 8);
	assert(a[2] == 10);
	assert(a[7] == 9);

	//parray<int> ah(new int[10], 10);
	//ah.push_back(12);
	//assert(ah[0] == 12);
	//ah.erase_element(12);
	//assert(ah.size() == 0);
	//printArray(ah);

	testArrayShallowCopy();

	testSwapMemory();

	testVArray();

	testPArray();

	testPageArray();

	printf("test array \t\tOK!\n");
}
