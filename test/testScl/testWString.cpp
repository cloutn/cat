#include "./testWString.h"

#include "scl/string.h"
#include "scl/math.h"

#include "scl/assert.h"

#include <stdio.h>

using scl::wstring;

void testWString_copy()
{
	wstring16 s1;
	s1.copy(L"a好b人c");
	assert(s1 == L"a好b人c");

	wstring<4> s2;
	s2.copy(L"lake");
	assert(s2 == L"lak");

	wstring16 s3;
	s3.copy(L"good", 3);
	assert(s3 == L"goo");
}

void testWString_append()
{
	wstring16 s1 = L"ab";
	s1.append(L"cd");
	assert(s1 == L"abcd");

	wstring<4> s2 = L"ab";
	s2.append(L"cd");
	assert(s2 == L"abc");

	wstring16 s3 = L"kf";
	s3.append(L"cd", 1);
	assert(s3 == L"kfc");

	wstring16 s4 = L"ab";
	s4.append(L'e');
	assert(s4 == L"abe");
}

void testWString_compare()
{
	wstring16 s1 = L"ab";
	assert(s1.compare(L"AB", true) == 0);
	assert(s1.compare(L"AB") != 0);

	wstring16 s2 = L"abcde";
	assert(s2.compare(L"abck", 3) == 0);
	assert(s2.compare(L"abck", 4) != 0);
	assert(s2.compare(L"ABCK", 3, true) == 0);
	assert(s2.compare(L"ABCK", 4, true) != 0);
}

void testWString_erase()
{
	wstring16 s1 = L"0123456";
	s1.erase();
	assert(s1 == L"");

	s1 = L"0123456";
	s1.erase(1);
	assert(s1 == L"0");

	s1 = L"0123456";
	s1.erase(1, 3);
	assert(s1 == L"0456");
}

void testWString_clear()
{
	wstring16 s1 = L"0123456";
	s1.clear();
	for (int i = 0; i < 16; ++i)
	{
		assert(s1[i] == 0);
	}
}

void testWString_format()
{
	wstring16 s1;
	s1.format(L"%d", 123456);
	assert(s1 == L"123456");

	wstring<4> s2;
	s2.format(L"%d", 123456);
	assert(s2 == L"123");
}

void testWString_format_append()
{
	wstring16 s1 = L"id = ";
	s1.format_append(L"%d", 123456);
	assert(s1 == L"id = 123456");

	wstring<4> s2 = L"0";
	s2.format_append(L"%d", 123456);
	assert(s2 == L"012");
}


void testWString_find()
{
	wstring16 s1 = L"1234";
	assert(s1.find(L"23") == 1);
	assert(s1.find(L"123") >= 0);
	assert(s1.find(L"124") < 0);
	assert(s1.find(L'3') == 2);
	assert(s1.find(L'5') < 0);

	wstring16 s2 = L"12341234";
	assert(s2.find_first_of(L"1234") == 0);
	assert(s2.find_last_of(L"1234") == 4);
	assert(s2.find_first_of(L'2') == 1);
	assert(s2.find_last_of(L'2') == 5);
	assert(s2.contains(L'4'));
	assert(!s2.contains(L'5'));
	assert(s2.contains(L"234"));
	assert(!s2.contains(L"235"));
}

//void testWString_find_line_end()
//{
//	wstring16 s1 = L"name	level\r\n";
//	s1.find_line_end(s1.c_str() + s1.length());
//}

void testWString_substr()
{
	wstring16 s1 = L"it is not good!";
	wstring16 s2;
	s1.substr(6, 3, s2.c_str(), s2.capacity());
	assert(s2 == L"not");
}


void testWString_replace()
{
	wstring32 s1 = L"it is bad bad thing?";
	s1.replace(L"bad", L"good");
	assert(s1 == L"it is good bad thing?");

	wstring32 s2 = L"it is bad bad thing?";
	s2.replace_all(L"bad", L"good");
	assert(s2 == L"it is good good thing?");

	//测试溢出问题
	int overflow_detector_before[10] = { 0 };
	wstring<8> s3 = L"1234567";
	int overflow_detector_after[10] = { 0 };
	for (int i = 0; i < 10; ++i)
	{
		overflow_detector_before[i] = 0x11 * i;
		overflow_detector_after	[i] = 0x11 * i;
	}
	s3.replace(L"34", L"987654321");
	assert(s3 == L"1298765");
	for (int i = 0; i < 10; ++i)
	{
		assert(overflow_detector_before	[i] == 0x11 * i);
		assert(overflow_detector_after	[i] == 0x11 * i);
	}
}

void testWString_insert()
{
	wstring32 s1 = L"it is bad thing";
	s1.insert(6, L"not ");
	assert(s1 == L"it is not bad thing");

	wstring32 s2 = L"it is bad thing";
	s2.insert(6, L'a');
	assert(s2 == L"it is abad thing");

	//测试溢出问题  在win32的release和linux下有效
	int overflow_detector_before[10] = { 0 };
	wstring<8> s3 = L"1234567";
	int overflow_detector_after[10] = { 0 };
	for (int i = 0; i < 10; ++i)
	{
		overflow_detector_before[i] = 0x11 * i;
		overflow_detector_after	[i] = 0x11 * i;
	}
	s3.insert(2, L"987654321");
	assert(s3 == L"1298765");
	for (int i = 0; i < 10; ++i)
	{
		assert(overflow_detector_before	[i] == 0x11 * i);
		assert(overflow_detector_after	[i] == 0x11 * i);
	}
}

void testWString_toupper_tolower()
{
	wstring32 s1 = L"it is bad thing";
	s1.toupper();
	assert(s1 == L"IT IS BAD THING");

	s1.tolower();
	assert(s1 == L"it is bad thing");
}


void testWString_trim()
{
	wstring32 s1 = L"			good	a	 		";
	s1.trim_left();
	assert(s1 == L"good	a	 		");

	wstring32 s2 = L"			good	a	 		";
	s2.trim_right();
	assert(s2 == L"			good	a");

	wstring32 s3 = L"			good	a	 		";
	s3.trim();
	assert(s3 == L"good	a");
}

void testWString_from_number()
{
	wstring32 s1;

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

void testWString_to_number()
{
	wstring32 s1 = L"-116";
	assert(s1.to_int() == -116);

	s1 = L"116";
	assert(s1.to_int() == 116);

	s1 = L"2147483649";
	assert(s1.to_uint() == static_cast<uint>(scl::pow(2, 31) + 1));

	s1 = L"123.456000";
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


void testWString_start_with_end_with()
{
	wstring128 s1;
	s1 = L"this is end";
	
	assert(s1.end_with(L"end"));
	assert(!s1.end_with(L" nd"));
	assert(s1.start_with(L"this"));
	assert(!s1.start_with(L"thi "));

	assert(s1.end_with(L"this is end"));
	assert(!s1.end_with(L""));
	assert(s1.start_with(L"this is end"));
	assert(!s1.start_with(L""));
	
	wstring128 s2;
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

	wstring128 s3;
	s3.clear();
	assert(s3.end_with(L""));
	assert(!s3.end_with(L"a"));
	assert(s3.start_with(L""));
	assert(!s3.start_with(L"a"));
}

void testWString()
{
	testWString_copy();

	testWString_append();

	testWString_compare();

	testWString_erase();

	testWString_clear();

	testWString_format();

	testWString_format_append();

	testWString_find();

	//testWString_find_line_end();

	testWString_substr();

	testWString_replace();

	testWString_insert();

	testWString_toupper_tolower();

	testWString_trim();

	testWString_from_number();

	testWString_to_number();

	testWString_start_with_end_with();

	printf("test wstring	\tOK!\n");
}


