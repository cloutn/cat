#include "testBitset.h"

#include "scl/type.h"
#include "scl/bitset.h"
#include "scl/assert.h"

#include <stdio.h>
#include <memory.h>

inline int mod_flag(int n)
{
	
}

void testBitset()
{
	//for (uint i = 0; i < 3737; ++i)
	//{
	//	//printf("%d % 8 = %d\n", i, (i & 0x0007));
	//	assert((i & 0x00000007) == (i % 8));
	//	assert(mod_flag(i) == ((i % 8) == 0 ? 0 : 1));
	//	assert(((i & 0xFFFFFFF8) >> 3) == (i / 8));
	//}
	unsigned char verify[2] = { 0 };
	scl::bitset<11> bs;
	assert(0 == ::memcmp(bs.flags(), verify, 2));

	////////////////////////////////////
	// set
	////////////////////////////////////
	bs.set(3);
	verify[0] = 0x10; verify[1] = 0;
	assert(0 == ::memcmp(bs.flags(), verify, 2));

	bs.set(9);
	verify[0] = 0x10; verify[1] = 0x40;
	assert(0 == ::memcmp(bs.flags(), verify, 2));

	bs.set(10);
	verify[0] = 0x10; verify[1] = 0x60;
	assert(0 == ::memcmp(bs.flags(), verify, 2));

	////////////////////////////////////
	// reset
	////////////////////////////////////
	bs.reset(9);
	verify[0] = 0x10; verify[1] = 0x20;
	assert(0 == ::memcmp(bs.flags(), verify, 2));

	bs.reset(3);
	verify[0] = 0x00; verify[1] = 0x20;
	assert(0 == ::memcmp(bs.flags(), verify, 2));

	bs.reset(10);
	verify[0] = 0x00; verify[1] = 0x00;
	assert(0 == ::memcmp(bs.flags(), verify, 2));

	printf("test bitset \t\tOK!\n");
}

