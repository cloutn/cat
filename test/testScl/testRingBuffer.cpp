
#include "testRingBuffer.h"

#include "scl/ring_buffer.h"
#include "scl/string.h"

#include <stdio.h>

//using scl::SizeSerializer;
using scl::ring_buffer;

const int TEST_BUFFER_LENGTH = 7;

bool checkBuffer(void* buffer, char* right)
{
	char cmp[TEST_BUFFER_LENGTH] = { 0 };
	for (int i = 0; i < TEST_BUFFER_LENGTH; ++i)
	{
		if (right[i] == '-')
		{
			cmp[i] = 0;
		}
		else
		{
			cmp[i] = right[i];
		}
	}

	return 0 == memcmp(buffer, cmp, TEST_BUFFER_LENGTH);
}

// void testRingBuffer()
//{
//	ring_buffer rb;	
//	rb.Init(TEST_BUFFER_LENGTH);
//	assert(!rb.isWrapped());
//	assert(rb.isFreeWrapped());
// 	char data[16] = "1234567890";
//	char out[TEST_BUFFER_LENGTH] = { 0 };
//
////	空buffer对应的比较字符串 "-------"
//
//	rb.write(data, 5);
//	assert(checkBuffer(rb.getBuffer(), "12345--"));
//	assert(rb.Length() == 5);
//	assert(!rb.isWrapped());
//	assert(rb.isFreeWrapped());
//
//	rb.read(out, 3);
//	assert(checkBuffer(rb.getBuffer(), "12345--"));
//	assert(rb.Length() == 2);
//	assert(!rb.isWrapped());
//	assert(rb.isFreeWrapped());
//
//	rb.write(data + 5, 2);
//	assert(checkBuffer(rb.getBuffer(), "1234567"));
//	assert(rb.Length() == 4);
//	assert(!rb.isWrapped());
//	assert(rb.isFreeWrapped());
//
//	rb.write(data + 5, 1);
//	assert(checkBuffer(rb.getBuffer(), "6234567"));
//	assert(rb.Length() == 5);
//	assert(rb.isWrapped());
//	assert(!rb.isFreeWrapped());
//
//	rb.write(data + 5, 2);
//	assert(checkBuffer(rb.getBuffer(), "6674567"));
//	assert(rb.Length() == 7);
//	assert(rb.isWrapped());
//	assert(!rb.isFreeWrapped());
//
//	rb.read(out, 6);
//	assert(checkBuffer(rb.getBuffer(), "6674567"));
//	assert(rb.Length() == 1);
//	assert(!rb.isWrapped());
//	assert(rb.isFreeWrapped());
//
//	rb.write(data, 6);
//	assert(checkBuffer(rb.getBuffer(), "5671234"));
//	assert(rb.Length() == 7);
//	assert(rb.isWrapped());
//	assert(!rb.isFreeWrapped());
//
//	printf("test ring_buffer \tOK!\n");
//}


 void testRingBuffer()
{
//	SizeSerializer sssss;

	ring_buffer rb;	
	rb.alloc(TEST_BUFFER_LENGTH);
	assert(!rb.is_wrapped());
 	char data[16] = "1234567890";
	char out[TEST_BUFFER_LENGTH] = { 0 };

//	空buffer对应的比较字符串 "-------"

	rb.write(data, 5);
	assert(checkBuffer(rb.get_buffer(), "12345--"));
	assert(rb.used() == 5);
	assert(!rb.is_wrapped());

	rb.read(out, 3);
	assert(checkBuffer(rb.get_buffer(), "12345--"));
	assert(rb.used() == 2);
	assert(!rb.is_wrapped());

	rb.write(data + 5, 2);
	assert(checkBuffer(rb.get_buffer(), "1234567"));
	assert(rb.used() == 4);
	assert(rb.is_wrapped());
	assert(!rb.is_free_wrapped());

	rb.write(data + 5, 1);
	assert(checkBuffer(rb.get_buffer(), "6234567"));
	assert(rb.used() == 5);
	assert(rb.is_wrapped());
	assert(!rb.is_free_wrapped());

	rb.write(data + 5, 1);
	assert(checkBuffer(rb.get_buffer(), "6634567"));
	assert(rb.used() == 6);
	assert(rb.is_wrapped());
	assert(!rb.is_free_wrapped());

	rb.read(out, 5);
	assert(checkBuffer(rb.get_buffer(), "6634567"));
	assert(rb.used() == 1);
	assert(!rb.is_wrapped());
	assert(rb.is_free_wrapped());

	rb.write(data, 5);
	assert(checkBuffer(rb.get_buffer(), "6612345"));
	assert(rb.used() == 6);
	assert(rb.is_wrapped());
	assert(!rb.is_free_wrapped());

	printf("test ring_buffer \tOK!\n");
}

