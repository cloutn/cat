
#include "testList.h"

#include "scl/list.h"
#include "scl/allocator.h"

#include <stdio.h>

using scl::list;
using scl::list_node;
using scl::single_allocator;

typedef list_node<int> Node;

 void testList()
{
	typedef single_allocator<list_node<int> > IntNodeAlloc;
	typedef list<int, IntNodeAlloc>			IntList;
	IntList ageList;
	//single_allocator<list_node<int> >::Singleton().Init(100);

	//ageList.SetAllocator(single_allocator<list_node<int> >::Singleton());

	//测试add与find
	for (int i = 0; i < 20; ++i)
	{
		ageList.push_back(i);
	}
	for (int j = 0; j < 20; ++j)
	{
		const IntList::iterator p = ageList.find(j);
		assert(p != ageList.end());
	}

	//测试addAtHead()和Head()
	ageList.push_front(999);
	assert(ageList.front() == 999);

	//测试Remove()
	ageList.remove(15);
	IntList::iterator pr = ageList.find(15);
	assert(pr == NULL);

	ageList.erase(ageList.begin());
	IntList::iterator p8 = ageList.find(999);
	assert(p8 == NULL);

	//测试insert()
	//头部插入
	ageList.insert(ageList.begin(), 77);
	assert(*(++ageList.begin()) == 77);

	//尾部插入
	ageList.insert(ageList.rbegin(), 88);
	assert(ageList.back() == 88);

	//测试insertBefore()
	//头部插入
	ageList.push_front(444);
	assert(ageList.front() == 444);

	//尾部插入
	ageList.insert_before(ageList.rbegin(), 555);
	assert(*(--ageList.rbegin()) == 555);

	//测试InsertAfter()
	//头部插入
	ageList.insert(ageList.begin(), 1111);
	assert(*(++ageList.begin()) == 1111);

	//尾部插入
	ageList.insert(ageList.rbegin(), 2222);
	assert(ageList.back() == 2222);


	//中间插入


	//测试const接口
	

	//ageList.Remove(23);
	//list_node<int>* pl = ageList.FindLast(20 + 19);
	//pl = pl;	//disable g++ warning
	//list_node<int>* pk = ageList.Find(22);
	//pk = pk; //disable g++ warning
	
	printf("test list \t\tOK!\n");
}

