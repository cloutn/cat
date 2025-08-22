#include "testTypeTraits.h"

#include "scl/type_traits.h"
#include "scl/varray.h"
#include "scl/assert.h"
#include "scl/array.h"
#include "scl/log.h"

#include <stdio.h>

void none_const_func(int* p)
{
	//printf("none_const_func\n");
}

typedef void (*test_handler_t)(int level, const char* const msg);

static void _testRemoveConst()
{
	int i = 0;
	const int* p = &i;
	none_const_func((scl::remove_const_t<scl::remove_pointer_t<const int*>>*)(p));

	scl::varray<int*> arr;
	arr.push_back(&i);

	int i2 = 1;
	const int* p2 = &i2;
	assert(!arr.contains(p2));
	//printf("array contains p2 = %d\n", arr.contains(p2));

	bool b1 = scl::is_function<test_handler_t>::value;
	bool b2 = scl::is_function<int*>::value;;
	bool b3 = scl::is_function<scl::remove_pointer_t<test_handler_t>>::value;

	assert(b1);
	assert(!b2);
	assert(b3);

	assert((scl::is_int<scl::conditional_t<true, int, float>>::value));
	assert((scl::is_float<scl::conditional_t<false, int, float>>::value));
	assert((scl::is_class<scl::array<int, 16> >::value));
	assert(scl::is_class<scl::log >::value);
	assert(!scl::is_class<const int*>::value);
	assert(scl::is_pointer<const int*>::value);
}


void testTypeTraits()
{
	_testRemoveConst();
}


