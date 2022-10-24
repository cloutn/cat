
#include "testThread.h"

#include "scl/type.h"
#include "scl/thread.h"

#include <stdio.h>

using scl::thread;

void* workThread(void* p, int* signal)
{
	printf("test thread \t\tOK!\n");
	return 0;
}

void testThread()
{
	static thread t;
	t.start(workThread);
}


