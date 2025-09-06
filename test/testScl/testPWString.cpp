#include "./testPWString.h"


#include "scl/string.h"
#include "scl/math.h"
#include "scl/assert.h"

#include <stdio.h>
#include <memory.h>

//wchar池，保证在一次测试过程中只调用一次系统的new
struct wchar_pool
{
	wchar	s[1024 * 4];
	int		free_index;
	wchar_pool() { memset(s, 0, sizeof(s)); free_index = 0; }

	wchar* get(const int n) { free_index += n; return s + free_index - n; }
};

wchar_pool* wpl = NULL;

#define NEW_CHAR(n) wpl->get(n), n

void testPWString_copy()
{	
	pwstring s1(NEW_CHAR(16));
	s1.copy(L"a好b人c");
	assert(s1 == L"a好b人c");

	pwstring s2(NEW_CHAR(4));
	s2.copy(L"lake");
	assert(s2 == L"lak");

	pwstring s3(NEW_CHAR(16));
	s3.copy(L"good", 3);
	assert(s3 == L"goo");
}

void testPWString_append()
{
	pwstring s1(NEW_CHAR(16), L"ab");
	s1.append(L"cd");
	assert(s1 == L"abcd");

	pwstring s2(NEW_CHAR(4), L"ab");
	s2.append(L"cd");
	assert(s2 == L"abc");

	pwstring s3(NEW_CHAR(16), L"kf");
	s3.append(L"cd", 1);
	assert(s3 == L"kfc");

	pwstring s4(NEW_CHAR(16), L"ab");
	s4.append('e');
	assert(s4 == L"abe");
}

void testPWString_compare()
{
	pwstring s1(NEW_CHAR(16), L"ab");
	assert(s1.compare(L"AB", true) == 0);
	assert(s1.compare(L"AB") != 0);

	pwstring s2(NEW_CHAR(16), L"abcde");
	assert(s2.compare(L"abck", 3) == 0);
	assert(s2.compare(L"abck", 4) != 0);
	assert(s2.compare(L"ABCK", 3, true) == 0);
	assert(s2.compare(L"ABCK", 4, true) != 0);
}

void testPWString_erase()
{
	pwstring s1(NEW_CHAR(16), L"0123456");
	s1.erase();
	assert(s1 == L"");

	s1 = L"0123456";
	s1.erase(1);
	assert(s1 == L"0");

	s1 = L"0123456";
	s1.erase(1, 3);
	assert(s1 == L"0456");
}

void testPWString_clear()
{
	pwstring s1(NEW_CHAR(16), L"0123456");
	s1.clear();
	for (int i = 0; i < 16; ++i)
	{
		assert(s1[i] == 0);
	}
}

void testPWString_format()
{
	pwstring s1(NEW_CHAR(16));
	s1.format(L"%d", 123456);
	assert(s1 == L"123456");

	pwstring s2(NEW_CHAR(4));
	s2.format(L"%d", 123456);
	assert(s2 == L"123");
}

void testPWString_format_append()
{
	pwstring s1(NEW_CHAR(16), L"id = ");
	s1.format_append(L"%d", 123456);
	assert(s1 == L"id = 123456");

	pwstring s2(NEW_CHAR(4), L"0");
	s2.format_append(L"%d", 123456);
	assert(s2 == L"012");
}


void testPWString_find()
{
	pwstring s1(NEW_CHAR(16), L"1234");
	assert(s1.find(L"23") == 1);
	assert(s1.find(L"123") >= 0);
	assert(s1.find(L"124") < 0);
	assert(s1.find('3') == 2);
	assert(s1.find('5') < 0);

	pwstring s2(NEW_CHAR(16), L"12341234");
	assert(s2.find_first_of(L"1234") == 0);
	assert(s2.find_last_of(L"1234") == 4);
	assert(s2.find_first_of('2') == 1);
	assert(s2.find_last_of('2') == 5);
	assert(s2.contains('4'));
	assert(!s2.contains('5'));
	assert(s2.contains(L"234"));
	assert(!s2.contains(L"235"));
}

//void testPWString_find_line_end()
//{
//	pwstring s1(NEW_CHAR(16), L"name	level\r\n");
//	s1.find_line_end(s1.c_str() + s1.length());
//}

void testPWString_substr()
{
	pwstring s1(NEW_CHAR(16), L"it is not good!");
	pwstring s2(NEW_CHAR(16));
	s1.substr(6, 3, s2.c_str(), s2.capacity());
	assert(s2 == L"not");
}


void testPWString_replace()
{
	pwstring s1(NEW_CHAR(32), L"it is bad bad thing?");
	s1.replace(L"bad", L"good");
	assert(s1 == L"it is good bad thing?");

	pwstring s2(NEW_CHAR(32), L"it is bad bad thing?");
	s2.replace_all(L"bad", L"good");
	assert(s2 == L"it is good good thing?");
}

void testPWString_insert()
{
	pwstring s1(NEW_CHAR(32), L"it is bad thing");
	s1.insert(6, L"not ");
	assert(s1 == L"it is not bad thing");

	pwstring s2(NEW_CHAR(32), L"it is bad thing");
	s2.insert(6, 'a');
	assert(s2 == L"it is abad thing");
}

void testPWString_toupper_tolower()
{
	pwstring s1(NEW_CHAR(32), L"it is bad thing");
	s1.toupper();
	assert(s1 == L"IT IS BAD THING");

	s1.tolower();
	assert(s1 == L"it is bad thing");
}


void testPWString_trim()
{
	pwstring s1(NEW_CHAR(32), L"			good	a	 		");
	s1.trim_left();
	assert(s1 == L"good	a	 		");

	pwstring s2(NEW_CHAR(32), L"			good	a	 		");
	s2.trim_right();
	assert(s2 == L"			good	a");

	pwstring s3(NEW_CHAR(32), L"			good	a	 		");
	s3.trim();
	assert(s3 == L"good	a");
}

void testPWString_from_number()
{
	pwstring s1(NEW_CHAR(32));

	s1.from_int(-116);
	assert(s1 == L"-116");

	s1.from_int(+116);
	assert(s1 == L"116");

	s1.from_uint(scl::pow(2, 31) + 1);
	assert(s1 == L"2147483649");

	s1.from_double(123.456);
	assert(s1 == L"123.456000");

	s1.from_int64(9223372036854775806ll);
	assert(s1 == L"9223372036854775806");

	s1.from_uint64(9223372036854775810ull);
	assert(s1 == L"9223372036854775810");
}

void testPWString_to_number()
{
	pwstring s1(NEW_CHAR(32), L"-116");
	assert(s1.to_int() == -116);

	s1 = L"116";
	assert(s1.to_int() == 116);

	s1 = L"2147483649";
	assert(s1.to_uint() == static_cast<uint>(scl::pow(2, 31) + 1));

	s1 = L"123.456";
	assert(s1.to_double() == 123.456);

	s1 = L"9223372036854775806";
	assert(s1.to_int64() == 9223372036854775806ll);

	s1 = L"9223372036854775810";
	assert(s1.to_uint64() == 9223372036854775810ull);

	s1 = L"0xAABBCCDD";
	assert(s1.to_hex() == 0xAABBCCDD);
	s1 = L"0xAAbbCCDD";
	assert(s1.to_hex() == 0xAABBCCDD);
	s1 = L"AAbbCCDD";
	assert(s1.to_hex() == 0xAABBCCDD);
}


void testPWString_start_with_end_with()
{	
	pwstring s1(NEW_CHAR(128));
	s1 = L"this is end";
	
	assert(s1.end_with(L"end"));
	assert(!s1.end_with(L" nd"));
	assert(s1.start_with(L"this"));
	assert(!s1.start_with(L"thi "));

	assert(s1.end_with(L"this is end"));
	assert(!s1.end_with(L""));
	assert(s1.start_with(L"this is end"));
	assert(!s1.start_with(L""));
	
	pwstring s2(NEW_CHAR(128));
	s2 = L"这是结 尾";
	assert(s2.end_with(L"这是结 尾"));
	assert(!s2.end_with(L""));
	assert(s2.start_with(L"这是结 尾"));
	assert(!s2.start_with(L""));

	assert(s2.end_with(L"尾"));
	assert(s2.end_with(L"结 尾"));
	assert(!s2.end_with(L"结尾"));
	assert(s2.start_with(L"这"));
	assert(s2.start_with(L"这是结 "));
	assert(!s2.end_with(L"这是 "));

	pwstring s3(NEW_CHAR(128));
	s3.clear();
	assert(s3.end_with(L""));
	assert(!s3.end_with(L"a"));
	assert(s3.start_with(L""));
	assert(!s3.start_with(L"a"));
}

void testPWString()
{
	wpl = new wchar_pool();

	testPWString_copy();

	testPWString_append();

	testPWString_compare();

	testPWString_erase();

	testPWString_clear();

	testPWString_format();

	testPWString_format_append();

	testPWString_find();

	//testPWString_find_line_end();

	testPWString_substr();

	testPWString_replace();

	testPWString_insert();

	testPWString_toupper_tolower();

	testPWString_trim();

	testPWString_from_number();

	testPWString_to_number();

	testPWString_start_with_end_with();

	delete wpl;

	printf("test pwstring \t\tOK!\n");
}

