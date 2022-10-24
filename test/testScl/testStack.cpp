
#include "scl/stack.h"
#include <stdio.h>

using scl::stack;

 void testStack()
{
	stack<int, 10> a;
	for (int i = 0; i < 10; ++i)
	{
		a.push(i);
	}
	assert(a.size() == 10);
	for (int j = 9; j >= 0; j--)
	{
		assert(j == a.top());
		const int q = a.pop();
		assert(q == j);
	}
	assert(a.size() == 0);

	printf("test stack \t\tOK!\n");
}

