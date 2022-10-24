
#include "testMemory.h"

#include "scl/array.h"
#include "scl/allocator.h"
#include "scl/pool.h"
#include <stdio.h>

using scl::single_allocator;
using scl::array;
using scl::pool;
using scl::stack;

class HeapTest
{
public:
	HeapTest()
	{
		p = new int;
	}
	~HeapTest()
	{
		delete p;
	}

private:
	char	a1;
	int16	a2;
	int		a3;
	int16	a4;
	int		a5;
	int*	p;
};

void testMemory()
{
	array<int*, 100> pointers;
	int temp = 0;

	single_allocator<int>::init(100);
	typedef single_allocator<int> IntAlloc;
	//SingleAllocator<int>& alloc = SingleAllocator<int>::Singleton();
	for (int i = 0; i < 100; ++i)
	{
		int* p = IntAlloc::alloc();
		pointers.push_back(p);
	}
	temp = pointers.size();

	for (int i = 0; i < 26; ++i)	//剩余26个
	{
		IntAlloc::free(pointers[i]);
		pointers.erase_fast(i);
	}
	temp = pointers.size();

	for (int i = 0; i < 17; ++i)	//剩余26 - 17 = 9个
	{
		int* p = IntAlloc::alloc();
		pointers.push_back(p);
	}
	temp = pointers.size();

	for (int i = 32; i < 32 + 29; ++i)	//剩余9 + 29 = 38个
	{
		IntAlloc::free(pointers[i]);
		pointers.erase_fast(i);
	}
	temp = pointers.size();

	for (int i = 0; i < 38; ++i)	//剩余0
	{
		int* p = IntAlloc::alloc();
		pointers.push_back(p);
	}
	assert(IntAlloc::is_full());
	
	for (int i = 0; i < pointers.size(); ++i)
		IntAlloc::free(pointers[i]);
	IntAlloc::release();

	typedef single_allocator<HeapTest> HeapTestAlloc;
	HeapTestAlloc::init(10);
	HeapTest* ph = HeapTestAlloc::alloc();
	ph = ph;

	HeapTestAlloc::free(ph);
	HeapTestAlloc::release();
}


class PoolObject
{
public:
	~PoolObject()
	{
		//delete[] p;
		//p = NULL;
	}
	void init(int i)
	{
		//p = new int[i];
	}
	int* p;
};

void testPool()
{
	pool<PoolObject> testPool;
	testPool.init(10);
	for (int i = 0; i < testPool.count(); ++i)
	{
		testPool[i].init(i * (i + 1));
	}

	stack<PoolObject*, 10> ps;
	for (int i = 0; i < 10; ++i)
	{
		ps.push(testPool.alloc());
	}

	while(!ps.empty())
	{
		testPool.free(ps.pop());
	}

	for (int i = 0; i < 7; ++i)
	{
		ps.push(testPool.alloc());
	}

	while(!ps.empty())
	{
		testPool.free(ps.pop());
	}
}

