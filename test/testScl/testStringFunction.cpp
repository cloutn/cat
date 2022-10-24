#include "testStringFunction.h"

#include "scl/string.h"
#include "scl/assert.h"


 void testStringFunction_char()
{
	//assert(str::len("good") == 4);
	//assert(str::nlen("good or bad", 4) == 4);
	//assert(str::nlen("good or bad", 128) == 11);
	//assert(str::ncmp("good or bad", "good and good", 4) == 0);
	//assert(str::ncmp("good or bad", "good and good", 128) > 0);
}

 void testStringFunction_wchar()
{

}

 void testStringFunction()
{
	testStringFunction_char();
	testStringFunction_wchar();
}
