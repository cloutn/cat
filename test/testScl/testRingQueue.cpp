#include "testRingQueue.h"

#include "scl/ring_queue.h"
#include "scl/vring_queue.h"
#include "scl/assert.h"
#include <stdio.h>

using scl::ring_queue;
using scl::vring_queue;

 void testRingQueue()
{
	ring_queue<int> rq;
	for (int i = 0; i < 10; ++i)
	{
		rq.push_back(i + 1);
	}
	
	for (int j = 0; j < 10; ++j)
	{
		int v = 0;
		rq.pop_front(v);
		assert(v == j + 1);
	}

	printf("test ring_queue \tOK!\n");
}


 void testVRingQueue()
{
	vring_queue<int> rq;
	rq.alloc(32);
	for (int i = 0; i < 10; ++i)
	{
		rq.push_back(i + 1);
	}
	
	for (int j = 0; j < 10; ++j)
	{
		int v = 0;
		rq.pop_front(v);
		assert(v == j + 1);
	}

	printf("test vring_queue \tOK!\n");
}




