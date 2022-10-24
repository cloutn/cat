
#include "testTime.h"

#include <stdio.h>

#include "scl/time.h"

 void testTime()
{	
	uint64 t1 = SCL_TICK;
	scl::sleep(333);
	uint64 t2 = SCL_TICK;

	//时间差的精度小于10毫秒
	int64 interval = t2 - t1;
	assert(interval - 333 <= 20);

	printf("test time \t\tOK!\n");

}

