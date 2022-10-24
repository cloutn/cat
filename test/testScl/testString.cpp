#include "./testString.h"

#include "scl/math.h"
#include "scl/string.h"
#include "scl/wstring.h"
#include "scl/assert.h"
#include "scl/string.h"
#include "scl/math.h"

#include <stdio.h>

using scl::string;
using scl::wstring;
using scl::wchar_to_ansi;
using scl::ansi_to_wchar;
using scl::Encoding_UTF8;
using scl::Encoding_GBK;

void test1()
{
	//测试compare，operator=
	wstring16 s1, s2;
	s1 = L"bbbb";
	s2 = L"BBBB";
	int q1 = s1.compare(s2);
	assert(q1 > 0);
	int q2 = s1.compare(s2, true);
	assert(q2 == 0);
	int q3 = s2.compare(s1);
	assert(q3 < 0);
	int q4 = s2.compare(s1, true);
	assert(q4 == 0);

	//测试clear
	wstring32 s3 = L"this is a wstring";
	assert(s3.length() == 17);
	s3.clear();
	assert(s3.length() == 0);
}

void test2()
{
	//测试format
	wstring<5> s3;
	s3.format(L"%d", 12345);
	assert(s3.length() == 4);
}

void test3()
{
	wstring16 s1;
	s1 = L"BBBB";

	wstring32 s2;
	assert(s2.length() == 0);
	s2 = L"bbbb";

	wstring<5> s3;
	s3 = L"1234";

	wstring<7> s4 = L"1234";

	//测试concat和operator+=
	s3 += L"bbaa";
	assert(s3.length() == 4);
	s3 += s1.c_str();
	assert(s3.length() == 4);

	s2 += L"bbaa";
	assert(s2.length() == 8);
	s2 += s1.c_str();
	assert(s2.length() == 12);

	s4 += L"bbaa";
	assert(s4.length() == 6);
}

void test4()
{
	//测试其他
	wstring16 s;

#ifdef _WIN32
	assert(s.sizeof_char() == 2);
#endif

#ifdef linux
	assert(s.sizeof_char() == 4);
#endif

	assert(s.capacity() == 15);
	assert(s.empty());
	s = L"123456";
	assert(s.length() == 6);
	assert(s.capacity() == 15);
	assert(!s.empty());
}

void test5()
{
	wstring128 s = L"let it be，顺其自然,let it be，顺其自然";
	int a1 = s.find(L't');
	assert(a1 == 2);
	int a2 = s.find(L"it");
	assert(a2 == 4);
	int a3 = s.find(L"顺");
	assert(a3 == 10);

	int a4 = s.find_last_of(L't');
	assert(a4 == 20);
	int a5 = s.find_last_of(L"it");
	assert(a5 == 19);
	int a6 = s.find_last_of(L"顺");
	assert(a6 == 25);
	assert(s[a6] == L'顺');

	assert(s.contains(L'b'));
	assert(!s.contains(L'a'));
	assert(s.contains(L'自'));
	assert(!s.contains(L'我'));
	assert(s.contains(L"it"));
	assert(!s.contains(L"ti"));
	assert(s.contains(L"其自"));
	assert(!s.contains(L"自其"));
}

void test6()
{
	//string64 s = L"abc,def,ghi.kui,dedf";
	//string64 output[4];
	//s.Split(L",.", output, 4);
	//assert(output[0] == L"abc");
	//assert(output[1] == L"def");
	//assert(output[2] == L"ghi");
	//assert(output[3] == L"kui");

	//array<wstring32, 4> outArray;
	//s.Split(L",", outArray);
	//assert(outArray.size() == 4);
	//assert(outArray[0] == L"abc");
	//assert(outArray[2] == L"ghi.kui");

	//string64 o333[4];
	//s.Split(L',', o333, 4);
	//assert(o333[0] == L"abc");
	//assert(o333[2] == L"ghi.kui");
};

void test7()
{
	wstring32 s = L"我是a的A拉b啊B啊Z奥迪z";
	s.toupper();
	assert(s[0] == L'我');
	assert(s[2] == L'A');
	assert(s[4] == L'A');
	assert(s[5] == L'拉');
	assert(s[12] == L'迪');
	assert(s[13] == L'Z');
	s.tolower();
	assert(s[0] == L'我');
	assert(s[2] == L'a');
	assert(s[4] == L'a');
	assert(s[5] == L'拉');
	assert(s[12] == L'迪');
	assert(s[13] == L'z');
}

void test8()
{
 	wstring32 s = L"我是a的A拉";
 	wstring32 out;
 	s.substr(1, 3, out.c_str(), out.max_size());
  	assert(out.length() == 3);
  	assert(out[0] == L'是');
  	assert(out[1] == L'a');
  	assert(out[2] == L'的');
 
 	wstring<3> out2;
 	s.substr(1, 5, out2.c_str(), out2.max_size());
  	assert(out2.length() == 2);
  	assert(out2[0] == L'是');
  	assert(out2[1] == L'a');
}

void test9()
{
 	wstring32 s = L"我A是a,567890";
 	s.erase(3, 3);
 	assert(s[3] == L'6');
 	s.erase(6, 4);
 	assert(s.length() == 6);
 	s.erase(9, 3);
 	assert(s[3] == L'6');
 	assert(s[5] == L'8');
 	assert(s.length() == 6);
}

void test10()
{
	wstring32 s1 = L"我是big好人";
	s1.replace(L"g好", L"ngo坏");
	assert(s1.length() == 9);
	assert(s1 == L"我是bingo坏人");
	
	wstring32 s2 = L"我是big好人";
	s2.replace(L"是big好", L"贼a");
	assert(s2.length() == 4);
	assert(s2[1] == L'贼');
	assert(s2[3] == L'人');

	wstring32 s3 = L"我是big好人";
	s3.replace(L"是b", L"贼a");
	assert(s3.length() == 7);
	assert(s3[1] == L'贼');
	assert(s3[2] == L'a');

	wstring32 s4 = L"1234567890";
	s4.replace(L"456", L"45678");
	assert(s4.length() == 12);
	assert(s4 == L"123456787890");
}

void test11()
{
	//测试在中间插入
	wstring32 s = L"ha我ha";
	s.insert(2, L"n并o不");
	assert(s.length() == 9);
	assert(s[2] == L'n');
	assert(s[5] == L'不');
	assert(s[6] == L'我');
	assert(s == L"han并o不我ha");

	//测试插入空串
	wstring32 s2 = L"ha我ha";
	s2.insert(3, L"");
	assert(s2 == L"ha我ha");

	//测试在末尾插入
	wstring32 s3 = L"ha我ha";
	s3.insert(5, L"个p");
	assert(s3 == L"ha我ha个p");

	//测试在头部插入
	wstring32 s4 = L"ha我ha";
	s4.insert(0, L"为h");
	assert(s4 == L"为hha我ha");
}

void test12()
{
	wstring32 s = L"\t  \n\r  \r\n a我k是个z   \t  \r\n\t";
	s.trim();
	assert(s == L"a我k是个z");
	assert(s.length() == 6);

	wstring32 s1 = L"\t  \n\r  \r\n a我k是个z   \t  \r\n\t";
	s1.trim_left();
	assert(s1 == L"a我k是个z   \t  \r\n\t");
	s1.trim_right();
	assert(s1 == L"a我k是个z");
}

void test13()
{
	wstring32 s = L"\t  \n\r  \r\n a我k是个z   \t  \r\n\t";
	s.trim();
	assert(s == L"a我k是个z");

	wstring32 s1 = L"\t  \n\r  \r\n a我k是个z   \t  \r\n\t";
	s1.trim_left();
	assert(s1 == L"a我k是个z   \t  \r\n\t");
	s1.trim_right();
	assert(s1 == L"a我k是个z");
}

void test14()
{
	wstring16 s1 = L"23";
	int i1 = s1.to_int();
	assert(i1 == 23);
	uint i2 = s1.to_uint();
	assert(i2 == 23);

	wstring16 s2 = L"23.554";
	double f1 = s2.to_double();
	assert(scl::absolute(f1 - 23.554) < 0.0001);

	wstring32 s3 = L"5300000000";
	long long i8 = s3.to_int64();
	assert(i8 == 5300000000ll);
	uint64 i9 = s3.to_uint64();
	assert(i9 == 5300000000ll);

	wstring16 s4;
	s4.from_int(23);
	assert(s4 == L"23");
	wstring16 s5;
	s5.from_uint(uint(3000000000u));
	assert(s5 == L"3000000000");
	wstring16 s6;
	s6.from_double(54.55);
	assert(s6.compare(L"54.55", 4) == 0);
	wstring16 s7;
	s7.from_int64(5200000000ll);
	assert(s7 == L"5200000000");
	wstring16 s8;
	s8.from_uint64(5200000000ll);
	assert(s8 == L"5200000000");
}

void test15()
{
	//常规测试
	wstring128 s;
	for (int i = 0; i < 10; ++i)
	{
		s.format_append(L"data=%d_", i + 1);
	}
	assert(s == L"data=1_data=2_data=3_data=4_data=5_data=6_data=7_data=8_data=9_data=10_");

	//测试string长度不够的情况
	wstring16 shortstring;
	for (int i = 0; i < 10; ++i)
	{
		shortstring.format_append(L"data=%d_", i + 1);
	}
	assert(shortstring == L"data=1_data=2_d");
}

//测试Ansistring
void test16()
{
	string32 as;
	as = "bbbaaa";
	assert(as == "bbbaaa");
}

void testEncoding()
{
	//测试转换为multiByte（utf8和gbk）
	wstring32 s = L"我是ab";
	char out[32] = { 0 };
	s.to_utf8(out, 32);
	assertf(out[0] == -26, 	"out[0] = %x", 	out[0]);
	assertf(out[1] == -120, "out[1] = %x", 	out[1]);
	assertf(out[2] == -111, "out[2] = %x", 	out[2]);
	assertf(out[3] == -26,	"out[3] = %x", 	out[3]);
	assertf(out[4] == -104,	"out[4] = %x", 	out[4]);
	assertf(out[5] == -81,	"out[5] = %x", 	out[5]);
	assertf(out[6] == 97,	"out[6] = %x", 	out[6]);
	assertf(out[7] == 98,	"out[7] = %x", 	out[7]);
	assertf(out[8] == 0, 	"out[8] = %x", 	out[8]);
	assertf(out[9] == 0, 	"out[9] = %x", 	out[9]);
	assertf(out[10] ==0, 	"out[10] = %x",	out[10]);

	wstring32 sutf16;
	sutf16.from_utf8(out);
	assert(sutf16 == L"我是ab");

	wstring32 s_to_gbk = L"我是ab";
	char gbk[32] = { 0 };
	s_to_gbk.to_gbk(gbk, 32);
	assertf(gbk[0] == static_cast<char>(static_cast<unsigned char>(0xCE)),	"gbk[0] = %hhX", 	gbk[0]);
	assertf(gbk[1] == static_cast<char>(static_cast<unsigned char>(0xD2)),	"gbk[1] = %hhX", 	gbk[1]);
	assertf(gbk[2] == static_cast<char>(static_cast<unsigned char>(0xCA)),	"gbk[2] = %hhX", 	gbk[2]);
	assertf(gbk[3] == static_cast<char>(static_cast<unsigned char>(0xC7)),	"gbk[3] = %hhX", 	gbk[3]);
	assertf(gbk[4] == static_cast<char>(static_cast<unsigned char>(0x61)),	"gbk[4] = %hhX", 	gbk[4]);
	assertf(gbk[5] == static_cast<char>(static_cast<unsigned char>(0x62)),	"gbk[5] = %hhX", 	gbk[5]);
	assertf(gbk[6] == 0,						"gbk[5] = %hhX", 	gbk[6]);

	wstring32 sgbk;
	sgbk.from_gbk(gbk);
	assert(sgbk == L"我是ab");
}

void test21()
{
	wstring32 s = L"abc 好人 bd_c";
	wchar* pContext = NULL;
	wchar* token = scl_wcstok(s.c_str(), L" ", &pContext);
	int i = 0;
	while (NULL != token)
	{
		if (i == 0)
		{
			assert(wcsncmp(token, L"abc", 32) == 0);
		}
		if (i == 1)
		{
			assert(wcsncmp(token, L"好人", 32) == 0);
		}
		if (i == 2)
		{
			assert(wcsncmp(token, L"bd_c", 32) == 0);
		}
		i++;
		token = scl_wcstok(NULL, L" ", &pContext);
	}
}

void test22()
{
	////测试Pstring在传入const char*的时候可以正确确定maxCount，
	////否则在进行compare操作的时候会由于maxCount为0而导致compare的结果总是“相等”
	//const wchar* p = L"12";
	//const pwstring s(p);
	//assert(s == L"12");
	//assert(s != L"21");

	//wchar sp1[] = L"111";
	//wchar sp2[] = L"222";
	//pwstring s1(sp1, sizeof(sp1) / sizeof(wchar));
	//pwstring s2(sp2, sizeof(sp2) / sizeof(wchar));
	//s1 = s2;
	//assert(s1 == s2);
	//assert(s1.c_str() == s2.c_str());

	//pwstring sNull;
	//assert(!(sNull == s1));
}

void test_wchar_to_ansi()
{
	//测试当目标缓冲区大小不足的情况
	{
		wchar	ws[3] = L"a我";
		char	s[4] = { 0 };
		wchar_to_ansi(s, 3, ws, -1, Encoding_UTF8);
		assert(strcmp(s, "a") == 0);
		s[3] = 0;
	}

	//测试当目标缓冲区大小不足的情况
	{
		char s[16] = "a我门去b";
		wchar ws[4] = { 0 };
#ifdef _WIN32
		ansi_to_wchar(ws, 4, s, -1, Encoding_GBK);
#endif
#ifdef linux
		ansi_to_wchar(ws, 4, s, -1, Encoding_UTF8);
#endif
		assert(wcscmp(ws, L"a我门") == 0);
	}

}

void testAnsiString_copy()
{
	string16 s1;
	s1.copy("a好b人c");
	assert(s1 == "a好b人c");

	string<4> s2;
	s2.copy("lake");
	assert(s2 == "lak");

	string16 s3;
	s3.copy("good", 3);
	assert(s3 == "goo");
}

void testAnsiString_append()
{
	string16 s1 = "ab";
	s1.append("cd");
	assert(s1 == "abcd");

	string<4> s2 = "ab";
	s2.append("cd");
	assert(s2 == "abc");

	string16 s3 = "kf";
	s3.append("cd", 1);
	assert(s3 == "kfc");

	string16 s4 = "ab";
	s4.append('e');
	assert(s4 == "abe");
}

void testAnsiString_compare()
{
	string16 s1 = "ab";
	assert(s1.compare("AB", true) == 0);
	assert(s1.compare("AB") != 0);

	string16 s2 = "abcde";
	assert(s2.compare("abck", 3) == 0);
	assert(s2.compare("abck", 4) != 0);
	assert(s2.compare("ABCK", 3, true) == 0);
	assert(s2.compare("ABCK", 4, true) != 0);
}

void testAnsiString_erase()
{
	string16 s1 = "0123456";
	s1.erase();
	assert(s1 == "");

	s1 = "0123456";
	s1.erase(1);
	assert(s1 == "0");

	s1 = "0123456";
	s1.erase(1, 3);
	assert(s1 == "0456");
}

void testAnsiString_clear()
{
	string16 s1 = "0123456";
	s1.clear();
	for (int i = 0; i < 16; ++i)
	{
		assert(s1[i] == 0);
	}
}

void testAnsiString_format()
{
	string16 s1;
	s1.format("%d", 123456);
	assert(s1 == "123456");

	string<4> s2;
	s2.format("%d", 123456);
	assert(s2 == "123");
}

void testAnsiString_format_append()
{
	string16 s1 = "id = ";
	s1.format_append("%d", 123456);
	assert(s1 == "id = 123456");

	string<4> s2 = "0";
	s2.format_append("%d", 123456);
	assert(s2 == "012");
}


void testAnsiString_find()
{
	string16 s1 = "1234";
	assert(s1.find("23") == 1);
	assert(s1.find("123") >= 0);
	assert(s1.find("124") < 0);
	assert(s1.find('3') == 2);
	assert(s1.find('5') < 0);

	string16 s2 = "12341234";
	assert(s2.find_first_of("1234") == 0);
	assert(s2.find_last_of("1234") == 4);
	assert(s2.find_first_of('2') == 1);
	assert(s2.find_last_of('2') == 5);
	assert(s2.contains('4'));
	assert(!s2.contains('5'));
	assert(s2.contains("234"));
	assert(!s2.contains("235"));
}

//void testAnsiString_find_line_end()
//{
//	string16 s1 = "name	level\r\n";
//	s1.find_line_end(s1.c_str() + s1.length());
//}

void testAnsiString_substr()
{
	string16 s1 = "it is not good!";
	string16 s2;
	s1.substr(6, 3, s2.c_str(), s2.capacity());
	assert(s2 == "not");
}


void testAnsiString_replace()
{
	string32 s1 = "it is bad bad thing?";
	s1.replace("bad", "good");
	assert(s1 == "it is good bad thing?");

	string32 s2 = "it is bad bad thing?";
	s2.replace_all("bad", "good");
	assert(s2 == "it is good good thing?");

	//测试溢出问题
	int overflow_detector_before[10] = { 0 };
	string<8> s3 = "1234567";
	int overflow_detector_after[10] = { 0 };
	for (int i = 0; i < 10; ++i)
	{
		overflow_detector_before[i] = 0x11 * i;
		overflow_detector_after	[i] = 0x11 * i;
	}
	s3.replace("34", "987654321");
	assert(s3 == "1298765");
	for (int i = 0; i < 10; ++i)
	{
		assert(overflow_detector_before	[i] == 0x11 * i);
		assert(overflow_detector_after	[i] == 0x11 * i);
	}

	string<4> s4 = "123";
	s4.replace("3", "aaa");
	assert(s4 == "12a");
}

void testAnsiString_insert()
{
	string32 s1 = "it is bad thing";
	s1.insert(6, "not ");
	assert(s1 == "it is not bad thing");

	string32 s2 = "it is bad thing";
	s2.insert(6, 'a');
	assert(s2 == "it is abad thing");

	//测试溢出问题  在win32的release和linux下有效
	int overflow_detector_before[10] = { 0 };
	string<8> s3 = "1234567";
	int overflow_detector_after[10] = { 0 };
	for (int i = 0; i < 10; ++i)
	{
		overflow_detector_before[i] = 0x11 * i;
		overflow_detector_after	[i] = 0x11 * i;
	}
	s3.insert(2, "987654321");
	assert(s3 == "1298765");
	for (int i = 0; i < 10; ++i)
	{
		assert(overflow_detector_before	[i] == 0x11 * i);
		assert(overflow_detector_after	[i] == 0x11 * i);
	}
}

void testAnsiString_toupper_tolower()
{
	string32 s1 = "it is bad thing";
	s1.toupper();
	assert(s1 == "IT IS BAD THING");

	s1.tolower();
	assert(s1 == "it is bad thing");
}


void testAnsiString_trim()
{
	string32 s1 = "			good	a	 		";
	s1.trim_left();
	assert(s1 == "good	a	 		");

	string32 s2 = "			good	a	 		";
	s2.trim_right();
	assert(s2 == "			good	a");

	string32 s3 = "			good	a	 		";
	s3.trim();
	assert(s3 == "good	a");
}

void testAnsiString_from_number()
{
	string32 s1;

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

void testAnsiString_to_number()
{
	string32 s1 = "-116";
	assert(s1.to_int() == -116);

	s1 = "116";
	assert(s1.to_int() == 116);

	s1 = "2147483649";
	assert(s1.to_uint() == static_cast<uint>(scl::pow(2, 31) + 1));

	s1 = "123.456000";
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

void testAnsiString_start_with_end_with()
{
	string128 s1;
	s1 = "this is end";
	
	assert(s1.end_with("end"));
	assert(!s1.end_with(" nd"));
	assert(s1.start_with("this"));
	assert(!s1.start_with("thi "));

	assert(s1.end_with("this is end"));
	assert(!s1.end_with(""));
	assert(s1.start_with("this is end"));
	assert(!s1.start_with(""));
	
	string128 s2;
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

	string128 s3;
	s3.clear();
	assert(s3.end_with(""));
	assert(!s3.end_with("a"));
	assert(s3.start_with(""));
	assert(!s3.start_with("a"));
}

void testString()
{
	testAnsiString_copy();

	testAnsiString_append();

	testAnsiString_compare();

	testAnsiString_erase();

	testAnsiString_clear();

	testAnsiString_format();

	testAnsiString_format_append();

	testAnsiString_find();

	//testAnsiString_find_line_end();

	testAnsiString_substr();

	testAnsiString_replace();

	testAnsiString_insert();

	testAnsiString_toupper_tolower();

	testAnsiString_trim();

	testAnsiString_from_number();

	testAnsiString_to_number();

	testAnsiString_start_with_end_with();

	printf("test string	\tOK!\n");
}

void old_testString()
{
	//测试compare，operator=
	test1();

	//测试format
	test2();

	//测试concat和operator+=
	test3();

	//测试其他
	test4();

	//测试Find相关
	test5();

	//测试Split相关
	test6();

	//测试ToUpper/ToLower
	test7();

	//测试Substring
	test8();

	//测试Remove
	test9();

	//测试Replace
	test10();

	//测试Insert
	test11();

	//测试Trim
	test12();

	//测试字符串换
	test13();

	//测试和数字的转换
	test14();

	//测试FormatConcat
	test15();

	//测试Ansistring
	test16();

	wstring16 bb = L"cdff";
	bb[bb.length() - 1] = 0;
	assert(bb.length() == 3);

	wstring<3> q = L"ab";
	assert(q.length() == 2);

	testEncoding();

	test21();

	test22();

	test_wchar_to_ansi();
	
	wstring32 sappend = L"我是ab";
	sappend.append(L"拉拉拉拉啊", 3);
	assert(sappend == L"我是ab拉拉拉");

	printf("test string	(old) \tOK!\n");
}


