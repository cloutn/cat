
#include "./testHeap.h"

#include <stdio.h>

#include "scl/math.h"
#include "scl/time.h"
#include "scl/heap.h"
#include "scl/assert.h"

#define CHECKP	assert(h->is_heap())
#define CHECK	assert(h.is_heap())
#define COMPARE_OBJECT_COUNT 10

using namespace scl;

rander rd;

typedef heap<int, 2048> heap2k;
typedef heap<int, 128> heap128;

class CompareObject
{
public:
	char x;

public:
	bool operator<(const CompareObject& other) { return x < other.x; }
};

void testBasic()
{
	heap2k* h = new heap2k;
	for (int i = 0; i < 1024; ++i)
	{
		int a = rd.rand(1, 1000);
		h->add(a);
	}

	CHECKP;

	//printf("heap : testBasic ok!\n");
	delete h;

}

void testBuildHeap(bool testHuge)
{
	if (testHuge)
	{
		heap<int, 1024 * 1024 * 128>* h = new heap<int, 1024 * 1024 * 128>;
		for (int i = 1024 * 1024 * 128; i > 0; --i)
			h->add_direct(i);
		assert(!h->is_heap());

		h->build();

		CHECKP;

		delete h;
	}
	else
	{
		heap<int, 1024 * 256>* h = new heap<int, 1024 * 256>;
		for (int i = 1024 * 256; i > 0; --i)
			h->add_direct(i);
		assert(!h->is_heap());

		h->build();

		CHECKP;

		delete h;
	}


	//printf("heap : testBuildHeap ok!\n");
}

bool check_tree(heap2k& h, int* check)
{
	for (int i = 0; i < h.size(); ++i)
	{
		if (check[i] != h[i])
			return false;
	}
	return true;
}

void testRemoveRandom()
{
	heap2k h;
	for (int i = 0; i < 1024; ++i)
	{
		h.add_direct(rd.rand(0, 32767));
	}
	h.build();
	assert(h.is_heap());

	while (h.size() > 0)
	{
		const int removeIndex = rd.rand(0, h.size() - 1);
		h.remove_at(removeIndex);
		assert(h.is_heap());
	}

	//printf("heap : testRemoveRandom ok!\n");
}

void testRemove()
{
	heap2k h;

	h.add(1);	
	h.add(2);h.add(20);
	h.add(3);h.add(4);h.add(30);	h.add(40);
	h.add(5);h.add(6);h.add(7);h.add(8);	h.add(50);h.add(60);	h.add(70);h.add(80);
	h.add(9);
	/*
			   1
		     /   \
		  2	       20
		 / \      /  \
		3   4    30   40
		/\  /\   /\   / \
	   5 6 7 8  50 60 70 80
	  /
	 9

	*/
	int check1[] = {1, 2, 20, 3, 4, 30, 40, 5, 6, 7, 8, 50, 60, 70, 80, 9};
	assert(check_tree(h, check1));
	CHECK;


	h.remove_at(6);
	/* after remove

			   1
		   /      \
		  2	       9
		 / \      /  \
		3   4    30   20
		/\  /\   /\   / \
	   5 6 7 8  50 60 70 80
	*/
	int check2[] = {1, 2, 9, 3, 4, 30, 20, 5, 6, 7, 8, 50, 60, 70, 80};
	assert(check_tree(h, check2));
	CHECK;

	h.remove_at(2);
	/* after remove

			   1
		   /      \
		  2	       20
		 / \      /  \
		3   4    30   70
		/\  /\   /\   / 
	   5 6 7 8  50 60 80 
	*/
	int check3[] = {1, 2, 20, 3, 4, 30, 70, 5, 6, 7, 8, 50, 60, 80};
	assert(check_tree(h, check3));
	CHECK;

	h.remove_at(1);
	/* after remove

			   1
		   /      \
		  3        20
		 /  \      /  \
		5   4    30   70
		/\   /\   /\    
	   80 6  7 8  50 60 
	*/
	int check4[] = {1, 3, 20, 5, 4, 30, 70, 80, 6, 7, 8, 50, 60};
	assert(check_tree(h, check4));
	CHECK;

	h.remove_at(12);
	/* after remove

			   1
		    /     \
		  3        20
		 /  \      /  \
		5   4     30   70
		/\   /\   /    
	   80 6  7 8 50 
	*/
	int check5[] = {1, 3, 20, 5, 4, 30, 70, 80, 6, 7, 8, 50};
	assert(check_tree(h, check5));
	CHECK;

	h.remove_at(0);
	/* after remove

			   3
		    /     \
		   4        20
		 /  \      /  \
		5    7     30   70
		/\   /\      
	   80 6 50 8  
	*/
	int check6[] = {3, 4, 20, 5, 7, 30, 70, 80, 6, 50, 8};
	assert(check_tree(h, check6));
	CHECK;

	h.remove_at(3);
	/* after remove

			   3
		    /     \
		   4        20
		 /  \      /  \
		6    7     30   70
		/\   /      
	   80 8 50 
	*/
	int check7[] = {3, 4, 20, 6, 7, 30, 70, 80, 8, 50};
	assert(check_tree(h, check7));
	CHECK;

	h.remove_at(8);
	/* after remove

			   3
		    /     \
		   4        20
		 /  \      /  \
		6    7     30   70
		/\         
	   80 50  
	*/
	int check8[] = {3, 4, 20, 6, 7, 30, 70, 80, 50};
	assert(check_tree(h, check8));
	CHECK;

	h.remove_at(1);
	/* after remove

			   3
		    /     \
		   6        20
		 /  \      /  \
		50    7     30   70
		/         
	   80   
	*/
	int check9[] = {3, 6, 20, 50, 7, 30, 70, 80};
	assert(check_tree(h, check9));
	CHECK;

	h.remove_at(4);
	/* after remove

			   3
		    /     \
		   6        20
		 /  \      /  \
		50   80   30   70
	*/
	int check10[] = {3, 6, 20, 50, 80, 30, 70};
	assert(check_tree(h, check10));
	CHECK;

	h.remove_at(2);
	/* after remove

			   3
		    /     \
		   6        30
		 /  \      /  
		50   80   70   
	*/
	int check11[] = {3, 6, 30, 50, 80, 70};
	assert(check_tree(h, check11));
	CHECK;

	h.remove_at(1);
	/* after remove

			   3
		    /     \
		   50      30
		 /  \      
		70   80      
	*/
	int check12[] = {3, 50, 30, 70, 80};
	assert(check_tree(h, check12));
	CHECK;

	h.remove_at(2);
	/* after remove

			   3
		    /     \
		   50      80
		 /        
		70  
	*/
	int check13[] = {3, 50, 80, 70};
	assert(check_tree(h, check13));
	CHECK;

	h.remove_at(1);
	/* after remove

			   3
		    /     \
		   70      80
	*/
	int check14[] = {3, 70, 80};
	assert(check_tree(h, check14));
	CHECK;

	h.remove_at(0);
	/* after remove

			   70
		    /     
		   80      
	*/
	int check15[] = {70, 80};
	assert(check_tree(h, check15));
	CHECK;

	h.remove_at(0);
	/* after remove
			   80
	*/
	int check16[] = {80};
	assert(check_tree(h, check16));
	CHECK;

	h.remove_at(0);
	/* after remove
			null
	*/
	assert(check_tree(h, NULL));
	CHECK;


	//printf("heap : testRemove ok!\n");

	return;
}


struct HeapElem
{
	float f;
	HeapElem() { f = 0; }
	HeapElem(float a) : f(a) {};

	bool operator==(const HeapElem& other)
	{
		return scl::float_equal(other.f, this->f);
	}
	bool operator<(const HeapElem& other)
	{
		return this->f < other.f;
	}
};
void testRemoveLast()
{
	heap<HeapElem, 128> h;
	h.add(HeapElem(1.0f));	
	h.add(HeapElem(2.0f));h.add(HeapElem(20.0f));
	//		1
	//    /   \
	//   2	   20

	h.remove_at(2);
	h.remove(2);

	//printf("heap : testRemoveLast ok!\n");
}

int testRobustness() //循环测试稳定性，注意，该函数不会返回，会一直循环
{
	int count = 0;
	int all = 0;
	while (1)
	{
		count++;
		if (count % 10000 == 0)
			++all;
		
		printf("======test count [%d] all[%d]======\n", count, all);

		testBasic();

		testRemove();

		testRemoveLast();

		testRemoveRandom();
	}
}

void testHeapCompareObject()
{
	heap<CompareObject, COMPARE_OBJECT_COUNT> h;
	CompareObject objs[COMPARE_OBJECT_COUNT];
	for (int i = 0; i < COMPARE_OBJECT_COUNT; ++i)
	{
		objs[i].x = COMPARE_OBJECT_COUNT - i;
		h.add(objs[i]);
	}
	
	CHECK;
}

void testHeap()
{
	testBasic();

	testBuildHeap(false);
	//testBuildHeap(true); //如果要进行大量数据的测试，参数传递true，否则只测试256k的数据

	testRemove();

	testRemoveLast();

	testRemoveRandom();

	testHeapCompareObject();

	//testRobustness();  //如无必要，不要进行鲁棒性测试，因为鲁棒性测试会一直循环，不会退出
	printf("test heap \t\tOK!\n");
}

