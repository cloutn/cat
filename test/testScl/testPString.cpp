#include "./testPString.h"

#include "scl/pstring.h"
#include "scl/math.h"
#include "scl/assert.h"
//#include "scl/string.h"

#include <stdio.h>

//char池，保证在一次测试过程中只调用一次系统的new
struct char_pool
{
	char	s[1024 * 4];
	int		free_index;
	char_pool() { memset(s, 0, sizeof(s)); free_index = 0; }

	char* get(const int n) { free_index += n; return s + free_index - n; }
};

char_pool* pl = NULL;

#define NEW_CHAR(n) pl->get(n), n

void testPAnsiString_copy()
{	
	pstring s1(NEW_CHAR(16));
	s1.copy("a好b人c");
	assert(s1 == "a好b人c");

	pstring s2(NEW_CHAR(4));
	s2.copy("lake");
	assert(s2 == "lak");

	pstring s3(NEW_CHAR(16));
	s3.copy("good", 3);
	assert(s3 == "goo");
}

void testPAnsiString_append()
{
	pstring s1(NEW_CHAR(16), "ab");
	s1.append("cd");
	assert(s1 == "abcd");

	pstring s2(NEW_CHAR(4), "ab");
	s2.append("cd");
	assert(s2 == "abc");

	pstring s3(NEW_CHAR(16), "kf");
	s3.append("cd", 1);
	assert(s3 == "kfc");

	pstring s4(NEW_CHAR(16), "ab");
	s4.append('e');
	assert(s4 == "abe");
}

void testPAnsiString_compare()
{
	pstring s1(NEW_CHAR(16), "ab");
	assert(s1.compare("AB", true) == 0);
	assert(s1.compare("AB") != 0);

	pstring s2(NEW_CHAR(16), "abcde");
	assert(s2.compare("abck", 3) == 0);
	assert(s2.compare("abck", 4) != 0);
	assert(s2.compare("ABCK", 3, true) == 0);
	assert(s2.compare("ABCK", 4, true) != 0);
}

void testPAnsiString_erase()
{
	pstring s1(NEW_CHAR(16), "0123456");
	s1.erase();
	assert(s1 == "");

	s1 = "0123456";
	s1.erase(1);
	assert(s1 == "0");

	s1 = "0123456";
	s1.erase(1, 3);
	assert(s1 == "0456");
}

void testPAnsiString_clear()
{
	pstring s1(NEW_CHAR(16), "0123456");
	s1.clear();
	for (int i = 0; i < 16; ++i)
	{
		assert(s1[i] == 0);
	}
}

void testPAnsiString_format()
{
	pstring s1(NEW_CHAR(16));
	s1.format("%d", 123456);
	assert(s1 == "123456");

	pstring s2(NEW_CHAR(4));
	s2.format("%d", 123456);
	assert(s2 == "123");
}

void testPAnsiString_format_append()
{
	pstring s1(NEW_CHAR(16), "id = ");
	s1.format_append("%d", 123456);
	assert(s1 == "id = 123456");

	pstring s2(NEW_CHAR(4), "0");
	s2.format_append("%d", 123456);
	assert(s2 == "012");
}


void testPAnsiString_find()
{
	pstring s1(NEW_CHAR(16), "1234");
	assert(s1.find("23") == 1);
	assert(s1.find("123") >= 0);
	assert(s1.find("124") < 0);
	assert(s1.find('3') == 2);
	assert(s1.find('5') < 0);

	pstring s2(NEW_CHAR(16), "12341234");
	assert(s2.find_first_of("1234") == 0);
	assert(s2.find_last_of("1234") == 4);
	assert(s2.find_first_of('2') == 1);
	assert(s2.find_last_of('2') == 5);
	assert(s2.contains('4'));
	assert(!s2.contains('5'));
	assert(s2.contains("234"));
	assert(!s2.contains("235"));
}

//void testPAnsiString_find_line_end()
//{
//	pstring s1(NEW_CHAR(16), "name	level\r\n");
//	s1.find_line_end(s1.c_str() + s1.length());
//}

void testPAnsiString_substr()
{
	pstring s1(NEW_CHAR(16), "it is not good!");
	pstring s2(NEW_CHAR(16));
	s1.substr(6, 3, s2.c_str(), s2.capacity());
	assert(s2 == "not");
}


void testPAnsiString_replace()
{
	pstring s1(NEW_CHAR(32), "it is bad bad thing?");
	s1.replace("bad", "good");
	assert(s1 == "it is good bad thing?");

	pstring s2(NEW_CHAR(32), "it is bad bad thing?");
	s2.replace_all("bad", "good");
	assert(s2 == "it is good good thing?");
}

void testPAnsiString_insert()
{
	pstring s1(NEW_CHAR(32), "it is bad thing");
	s1.insert(6, "not ");
	assert(s1 == "it is not bad thing");

	pstring s2(NEW_CHAR(32), "it is bad thing");
	s2.insert(6, 'a');
	assert(s2 == "it is abad thing");
}

void testPAnsiString_toupper_tolower()
{
	pstring s1(NEW_CHAR(32), "it is bad thing");
	s1.toupper();
	assert(s1 == "IT IS BAD THING");

	s1.tolower();
	assert(s1 == "it is bad thing");
}


void testPAnsiString_trim()
{
	pstring s1(NEW_CHAR(32), "			good	a	 		");
	s1.trim_left();
	assert(s1 == "good	a	 		");

	pstring s2(NEW_CHAR(32), "			good	a	 		");
	s2.trim_right();
	assert(s2 == "			good	a");

	pstring s3(NEW_CHAR(32), "			good	a	 		");
	s3.trim();
	assert(s3 == "good	a");
}

void testPAnsiString_from_number()
{
	pstring s1(NEW_CHAR(32));

	s1.from_int(-116);
	assert(s1 == "-116");

	s1.from_int(+116);
	assert(s1 == "116");

	s1.from_uint(scl::pow(2, 31) + 1);
	assert(s1 == "2147483649");

	s1.from_double(123.456);
	assert(s1 == "123.456000");

	s1.from_int64(9223372036854775806ll);
	assert(s1 == "9223372036854775806");

	s1.from_uint64(9223372036854775810ull);
	assert(s1 == "9223372036854775810");
}

void testPAnsiString_to_number()
{
	pstring s1(NEW_CHAR(32), "-116");
	assert(s1.to_int() == -116);

	s1 = "116";
	assert(s1.to_int() == 116);

	s1 = "2147483649";
	assert(s1.to_uint() == static_cast<uint>(scl::pow(2, 31) + 1));

	s1 = "123.456";
	assert(s1.to_double() == 123.456);

	s1 = "9223372036854775806";
	assert(s1.to_int64() == 9223372036854775806ll);

	s1 = "9223372036854775810";
	assert(s1.to_uint64() == 9223372036854775810ull);

	s1 = "0xAABBCCDD";
	assert(s1.to_hex() == 0xAABBCCDD);
	s1 = "0xAAbbCCDD";
	assert(s1.to_hex() == 0xAABBCCDD);
	s1 = "AAbbCCDD";
	assert(s1.to_hex() == 0xAABBCCDD);
}

void testPAnsiString_start_with_end_with()
{	
	pstring s1(NEW_CHAR(128));
	s1 = "this is end";
	
	assert(s1.end_with("end"));
	assert(!s1.end_with(" nd"));
	assert(s1.start_with("this"));
	assert(!s1.start_with("thi "));

	assert(s1.end_with("this is end"));
	assert(!s1.end_with(""));
	assert(s1.start_with("this is end"));
	assert(!s1.start_with(""));
	
	pstring s2(NEW_CHAR(128));
	s2 = "这是结 尾";
	assert(s2.end_with("这是结 尾"));
	assert(!s2.end_with(""));
	assert(s2.start_with("这是结 尾"));
	assert(!s2.start_with(""));

	assert(s2.end_with("尾"));
	assert(s2.end_with("结 尾"));
	assert(!s2.end_with("结尾"));
	assert(s2.start_with("这"));
	assert(s2.start_with("这是结 "));
	assert(!s2.end_with("这是 "));

	pstring s3(NEW_CHAR(128));
	s3.clear();
	assert(s3.end_with(""));
	assert(!s3.end_with("a"));
	assert(s3.start_with(""));
	assert(!s3.start_with("a"));
}


void testPString()
{
	pl = new char_pool();

	testPAnsiString_copy();

	testPAnsiString_append();

	testPAnsiString_compare();

	testPAnsiString_erase();

	testPAnsiString_clear();

	testPAnsiString_format();

	testPAnsiString_format_append();

	testPAnsiString_find();

	//testPAnsiString_find_line_end();

	testPAnsiString_substr();

	testPAnsiString_replace();

	testPAnsiString_insert();

	testPAnsiString_toupper_tolower();

	testPAnsiString_trim();

	testPAnsiString_from_number();

	testPAnsiString_to_number();

	testPAnsiString_start_with_end_with();

	delete pl;

	printf("test pstring \t\tOK!\n");
}





