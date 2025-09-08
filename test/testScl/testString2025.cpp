#include "./testString2025.h"

#include "scl/string.h"
#include "scl/math.h"
#include "scl/assert.h"

#include <stdio.h>
#include <stdarg.h>

// 日志输出函数实现
void mylog(const char* format, ...) {
	va_list args;
	va_start(args, format);
	//vprintf(format, args);
	va_end(args);
}

void mywlog(const wchar_t* format, ...) {
	va_list args;
	va_start(args, format);
	//vwprintf(format, args);
	va_end(args);
}

//#include <stdio.h>
//#include <assert.h>
//#include <string.h>

using scl::vstring;
using scl::vwstring;

// 测试函数声明
void test_string32_basic_operations();
void test_string32_string_operations();
void test_string32_conversions();
void test_string32_operators();
void test_wstring32_basic_operations();
void test_wstring32_string_operations();
void test_wstring32_conversions();
void test_wstring32_operators();
void test_pstring_basic_operations();
void test_pstring_string_operations();
void test_pstring_conversions();
void test_pstring_operators();
void test_pwstring_basic_operations();
void test_pwstring_string_operations();
void test_pwstring_conversions();
void test_pwstring_operators();

// 测试转换方法
void test_string_conversion_pstring();
void test_wstring_conversion_to_pwstring();

// 测试编码转换接口
void test_encoding_conversion_interfaces();

// 测试 scanf 和 reserve 接口
void test_scanf_and_reserve_interfaces();

void test_string32_basic_operations()
{
	mylog("Testing string32 basic operations...\n");
	
	// 构造和基本属性
	string32 s1;
	assert(s1.empty());
	assert(s1.length() == 0);
	assert(s1.capacity() == 31);
	assert(s1.max_size() == 32);
	
	// copy 方法
	s1.copy("Hello World");
	assert(!s1.empty());
	assert(s1.length() == 11);
	mylog("  copy: %s\n", s1.c_str());
	assert(strcmp(s1.c_str(), "Hello World") == 0);
	
	// copy 指定长度
	string32 s2;
	s2.copy("Hello World", 5);
	mylog("  copy(5): %s\n", s2.c_str());
	assert(s2.length() == 5);
	assert(strcmp(s2.c_str(), "Hello") == 0);
	
	// clear
	s2.clear();
	assert(s2.empty());
	
	// append
	s2.append("Test");
	s2.append(" String");
	mylog("  append: %s\n", s2.c_str());
	assert(strcmp(s2.c_str(), "Test String") == 0);
	
	// append 字符
	s2.append('!');
	mylog("  append char: %s\n", s2.c_str());
	assert(strcmp(s2.c_str(), "Test String!") == 0);
	
	// substract (删除末尾字符)
	string32 s3;
	s3.copy("abcdef");
	s3.substract(2);
	mylog("  substract(2): %s\n", s3.c_str());
	assert(s3.length() == 4);
	assert(strcmp(s3.c_str(), "abcd") == 0);
	
	mylog("Basic operations passed\n");
}

void test_string32_string_operations()
{
	mylog("Testing string32 string operations...\n");
	
	char* ttss1 = "Hellow";
	char* ttss2 = "hellow";

	int k =::strncmp(ttss1, ttss2, 32);

	string32 s1, s2, s3;
	s1.copy("Hello World");
	s2.copy("hello world");
	s3.copy("Test String");
	
	// 比较操作
	assert(s1.compare("Hello World") == 0);
	assert(s1.compare("hello world", true) == 0);  // 忽略大小写
	mylog("  compare: %d\n", s1.compare(s2.c_str()));
	assert(s1.compare(s2.c_str()) > 0);  // "Hello World" > "hello world" (ASCII)
	
	// 查找操作
	int pos = s1.find('o');
	mylog("  find 'o': position %d\n", pos);
	assert(pos == 4);
	
	pos = s1.find("World");
	mylog("  find 'World': position %d\n", pos);
	assert(pos == 6);
	
	pos = s1.find_last_of('o');
	mylog("  find_last_of 'o': position %d\n", pos);
	assert(pos == 7);
	
	// contains
	assert(s1.contains('H'));
	assert(s1.contains("World"));
	assert(!s1.contains("xyz"));
	
	// start_with 和 end_with
	assert(s1.start_with("Hello", false));
	assert(s1.start_with("hello", true));  // 忽略大小写
	assert(s1.end_with("World", false));
	assert(s1.end_with("world", true));  // 忽略大小写
	
	// replace
	string32 s4;
	s4.copy("Hello World Hello");
	s4.replace("Hello", "Hi");
	mylog("  replace first: %s\n", s4.c_str());
	assert(strcmp(s4.c_str(), "Hi World Hello") == 0);
	
	s4.copy("Hello World Hello");
	int count = s4.replace_all("Hello", "Hi");
	mylog("  replace_all (count=%d): %s\n", count, s4.c_str());
	assert(count == 2);
	assert(strcmp(s4.c_str(), "Hi World Hi") == 0);
	
	// insert
	string32 s5;
	s5.copy("Hello World");
	s5.insert(6, "Beautiful ");
	mylog("  insert: %s\n", s5.c_str());
	assert(strcmp(s5.c_str(), "Hello Beautiful World") == 0);
	
	s5.insert(0, 'X');
	mylog("  insert char: %s\n", s5.c_str());
	assert(strcmp(s5.c_str(), "XHello Beautiful World") == 0);
	
	// erase
	string32 s6;
	s6.copy("Hello World");
	s6.erase(5, 1);  // 删除空格
	mylog("  erase: %s\n", s6.c_str());
	assert(strcmp(s6.c_str(), "HelloWorld") == 0);
	
	// substr
	char sub[32];
	s1.substr(6, 5, sub, 32);
	mylog("  substr(6,5): %s\n", sub);
	assert(strcmp(sub, "World") == 0);
	
	mylog("String operations passed\n");
}

void test_string32_conversions()
{
	mylog("Testing string32 conversions and formatting...\n");
	
	// 数值转换 (from_xxx)
	string32 s1;
	s1.from_int(12345);
	mylog("  from_int: %s\n", s1.c_str());
	assert(strcmp(s1.c_str(), "12345") == 0);
	
	s1.from_double(123.456);
	mylog("  from_double: %s\n", s1.c_str());
	assert(strstr(s1.c_str(), "123.456") != NULL);
	
	s1.from_int64(9876543210LL);
	mylog("  from_int64: %s\n", s1.c_str());
	assert(strcmp(s1.c_str(), "9876543210") == 0);
	
	// 数值解析 (to_xxx)
	string32 s2;
	s2.copy("42");
	int val = s2.to_int(0, 10);
	mylog("  to_int: %d\n", val);
	assert(val == 42);
	
	s2.copy("3.14159");
	double dval = s2.to_double(0.0);
	mylog("  to_double: %f\n", dval);
	assert(dval > 3.14 && dval < 3.15);
	
	s2.copy("FF");
	uint hex_val = s2.to_hex(0);
	mylog("  to_hex: %u\n", hex_val);
	assert(hex_val == 255);
	
	s2.copy("true");
	bool bval = s2.to_bool(false);
	mylog("  to_bool: %d\n", bval);
	assert(bval == true);
	
	// 格式化
	string32 s3;
	s3.format("Number: %d, Float: %.2f", 42, 3.14);
	mylog("  format: %s\n", s3.c_str());
	assert(strcmp(s3.c_str(), "Number: 42, Float: 3.14") == 0);
	
	s3.format_append(", String: %s", "test");
	mylog("  format_append: %s\n", s3.c_str());
	assert(strcmp(s3.c_str(), "Number: 42, Float: 3.14, String: test") == 0);
	
	// 大小写转换
	string32 s4;
	s4.copy("Hello World");
	s4.toupper();
	mylog("  toupper: %s\n", s4.c_str());
	assert(strcmp(s4.c_str(), "HELLO WORLD") == 0);
	
	s4.tolower();
	mylog("  tolower: %s\n", s4.c_str());
	assert(strcmp(s4.c_str(), "hello world") == 0);
	
	// 单个字符大小写转换
	s4.copy("hello");
	s4.toupper(0);
	mylog("  toupper(0): %s\n", s4.c_str());
	assert(strcmp(s4.c_str(), "Hello") == 0);
	
	// 去空白字符
	string32 s5;
	s5.copy("  Hello World  ");
	int trimmed = s5.trim();
	mylog("  trim (removed %d chars): '%s'\n", trimmed, s5.c_str());
	assert(trimmed == 4);
	assert(strcmp(s5.c_str(), "Hello World") == 0);
	
	s5.copy("  Hello World  ");
	s5.trim_left();
	mylog("  trim_left: '%s'\n", s5.c_str());
	assert(strcmp(s5.c_str(), "Hello World  ") == 0);
	
	s5.copy("  Hello World  ");
	s5.trim_right();
	mylog("  trim_right: '%s'\n", s5.c_str());
	assert(strcmp(s5.c_str(), "  Hello World") == 0);
	
	mylog("Conversions and formatting passed\n");
}

void test_string32_operators()
{
	mylog("Testing string32 operators...\n");
	
	string32 s1, s2, s3;
	
	// 赋值操作符
	s1 = "Hello";
	s2 = s1;
	s3 = 42;
	mylog("  assignment: s1='%s', s2='%s', s3='%s'\n", s1.c_str(), s2.c_str(), s3.c_str());
	assert(strcmp(s1.c_str(), "Hello") == 0);
	assert(strcmp(s2.c_str(), "Hello") == 0);
	assert(strcmp(s3.c_str(), "42") == 0);
	
	// += 操作符
	s1 += " World";
	s1 += '!';
	s1 += s2;
	mylog("  += operator: %s\n", s1.c_str());
	assert(strcmp(s1.c_str(), "Hello World!Hello") == 0);
	
	// 比较操作符
	s1.copy("ABC");
	s2.copy("ABC");
	s3.copy("XYZ");
	
	assert(s1 == s2);
	assert(s1 != s3);
	assert(s1 < s3);
	assert(s3 > s1);
	assert(s1 == "ABC");
	assert(s1 != "XYZ");
	
	// [] 操作符
	s1.copy("Hello");
	assert(s1[0] == 'H');
	assert(s1[4] == 'o');
	s1[0] = 'h';
	mylog("  [] operator: %s\n", s1.c_str());
	assert(strcmp(s1.c_str(), "hello") == 0);
	
	mylog("Operators passed\n");
}

void test_wstring32_basic_operations()
{
	mylog("Testing wstring32 basic operations...\n");
	
	// 构造和基本属性
	wstring32 ws1;
	assert(ws1.empty());
	assert(ws1.length() == 0);
	assert(ws1.capacity() == 31);
	assert(ws1.max_size() == 32);
	
	// copy 方法
	ws1.copy(L"Hello World");
	assert(!ws1.empty());
	assert(ws1.length() == 11);
	mywlog(L"  copy: %ls\n", ws1.c_str());
	assert(wcscmp(ws1.c_str(), L"Hello World") == 0);
	
	// copy 指定长度
	wstring32 ws2;
	ws2.copy(L"Hello World", 5);
	mywlog(L"  copy(5): %ls\n", ws2.c_str());
	assert(ws2.length() == 5);
	assert(wcscmp(ws2.c_str(), L"Hello") == 0);
	
	// clear
	ws2.clear();
	assert(ws2.empty());
	
	// append
	ws2.append(L"Test");
	ws2.append(L" String");
	mywlog(L"  append: %ls\n", ws2.c_str());
	assert(wcscmp(ws2.c_str(), L"Test String") == 0);
	
	// append 字符
	ws2.append(L'!');
	mywlog(L"  append char: %ls\n", ws2.c_str());
	
	// substract
	wstring32 ws3;
	ws3.copy(L"abcdef");
	ws3.substract(2);
	mywlog(L"  substract(2): %ls\n", ws3.c_str());
	assert(ws3.length() == 4);
	
	mylog("Basic operations passed\n");
}

void test_wstring32_string_operations()
{
	mylog("Testing wstring32 string operations...\n");
	
	wstring32 ws1, ws2, ws3;
	ws1.copy(L"Hello World");
	ws2.copy(L"hello world");
	ws3.copy(L"Test String");
	
	// 比较操作
	assert(ws1.compare(L"Hello World") == 0);
	assert(ws1.compare(L"hello world", true) == 0);  // 忽略大小写
	mylog("  compare result: %d\n", ws1.compare(ws2.c_str()));
	
	// 查找操作
	int pos = ws1.find(L'o');
	mylog("  find 'o': position %d\n", pos);
	assert(pos == 4);
	
	pos = ws1.find(L"World");
	mylog("  find 'World': position %d\n", pos);
	assert(pos == 6);
	
	pos = ws1.find_last_of(L'o');
	mylog("  find_last_of 'o': position %d\n", pos);
	assert(pos == 7);
	
	// contains
	assert(ws1.contains(L'H'));
	assert(ws1.contains(L"World"));
	assert(!ws1.contains(L"xyz"));
	
	// start_with 和 end_with
	assert(ws1.start_with(L"Hello", false));
	assert(ws1.start_with(L"hello", true));  // 忽略大小写
	assert(ws1.end_with(L"World", false));
	assert(ws1.end_with(L"world", true));  // 忽略大小写
	
	// replace
	wstring32 ws4;
	ws4.copy(L"Hello World Hello");
	ws4.replace(L"Hello", L"Hi");
	mywlog(L"  replace first: %ls\n", ws4.c_str());
	
	ws4.copy(L"Hello World Hello");
	int count = ws4.replace_all(L"Hello", L"Hi");
	mywlog(L"  replace_all (count=%d): %ls\n", count, ws4.c_str());
	
	// insert
	wstring32 ws5;
	ws5.copy(L"Hello World");
	ws5.insert(6, L"Beautiful ");
	mywlog(L"  insert: %ls\n", ws5.c_str());
	
	ws5.insert(0, L'X');
	mywlog(L"  insert char: %ls\n", ws5.c_str());
	
	// erase
	wstring32 ws6;
	ws6.copy(L"Hello World");
	ws6.erase(5, 1);  // 删除空格
	mywlog(L"  erase: %ls\n", ws6.c_str());
	
	// substr
	wchar_t sub[32];
	ws1.substr(6, 5, sub, 32);
	mywlog(L"  substr(6,5): %ls\n", sub);
	
	mylog("String operations passed\n");
}

void test_wstring32_conversions()
{
	mylog("Testing wstring32 conversions and formatting...\n");
	
	// 数值转换 (from_xxx)
	wstring32 ws1;
	ws1.from_int(12345);
	mywlog(L"  from_int: %ls\n", ws1.c_str());
	
	ws1.from_double(123.456);
	mywlog(L"  from_double: %ls\n", ws1.c_str());
	
	ws1.from_int64(9876543210LL);
	mywlog(L"  from_int64: %ls\n", ws1.c_str());
	
	// 数值解析 (to_xxx)
	wstring32 ws2;
	ws2.copy(L"42");
	int val = ws2.to_int(0, 10);
	mylog("  to_int: %d\n", val);
	assert(val == 42);
	
	ws2.copy(L"3.14159");
	double dval = ws2.to_double(0.0);
	mylog("  to_double: %f\n", dval);
	
	ws2.copy(L"FF");
	uint hex_val = ws2.to_hex(0);
	mylog("  to_hex: %u\n", hex_val);
	assert(hex_val == 255);
	
	ws2.copy(L"true");
	bool bval = ws2.to_bool(false);
	mylog("  to_bool: %d\n", bval);
	assert(bval == true);
	
	// 格式化
	wstring32 ws3;
	ws3.format(L"Number: %d, Float: %.2f", 42, 3.14);
	mywlog(L"  format: %ls\n", ws3.c_str());
	
	ws3.format_append(L", String: %ls", L"test");
	mywlog(L"  format_append: %ls\n", ws3.c_str());
	
	// 大小写转换
	wstring32 ws4;
	ws4.copy(L"Hello World");
	ws4.toupper();
	mywlog(L"  toupper: %ls\n", ws4.c_str());
	
	ws4.tolower();
	mywlog(L"  tolower: %ls\n", ws4.c_str());
	
	// 单个字符大小写转换
	ws4.copy(L"hello");
	ws4.toupper(0);
	mywlog(L"  toupper(0): %ls\n", ws4.c_str());
	
	// 去空白字符
	wstring32 ws5;
	ws5.copy(L"  Hello World  ");
	int trimmed = ws5.trim();
	mywlog(L"  trim (removed %d chars): '%ls'\n", trimmed, ws5.c_str());
	
	ws5.copy(L"  Hello World  ");
	ws5.trim_left();
	mywlog(L"  trim_left: '%ls'\n", ws5.c_str());
	
	ws5.copy(L"  Hello World  ");
	ws5.trim_right();
	mywlog(L"  trim_right: '%ls'\n", ws5.c_str());
	
	mylog("Conversions and formatting passed\n");
}

void test_wstring32_operators()
{
	mylog("Testing wstring32 operators...\n");
	
	wstring32 ws1, ws2, ws3;
	
	// 赋值操作符
	ws1 = L"Hello";
	ws2 = ws1;
	ws3 = 42;
	mywlog(L"  assignment: ws1='%ls', ws2='%ls', ws3='%ls'\n", ws1.c_str(), ws2.c_str(), ws3.c_str());
	
	// += 操作符
	ws1 += L" World";
	ws1 += L'!';
	ws1 += ws2;
	mywlog(L"  += operator: %ls\n", ws1.c_str());
	
	// 比较操作符
	ws1.copy(L"ABC");
	ws2.copy(L"ABC");
	ws3.copy(L"XYZ");
	
	assert(ws1 == ws2);
	assert(ws1 != ws3);
	assert(ws1 < ws3);
	assert(ws3 > ws1);
	assert(ws1 == L"ABC");
	assert(ws1 != L"XYZ");
	
	// [] 操作符
	ws1.copy(L"Hello");
	assert(ws1[0] == L'H');
	assert(ws1[4] == L'o');
	ws1[0] = L'h';
	mywlog(L"  [] operator: %ls\n", ws1.c_str());
	
	mylog("Operators passed\n");
}

void test_pstring_basic_operations()
{
	mylog("Testing pstring basic operations...\n");
	
	// 创建缓冲区
	char buffer[100];
	memset(buffer, 0, sizeof(buffer));
	
	// 使用新的构造函数初始化
	pstring ps(buffer, 100);
	
	// 基本属性测试
	assert(ps.empty());
	assert(ps.length() == 0);
	assert(ps.capacity() == 99);
	assert(ps.max_size() == 100);
	
	// copy 方法
	ps.copy("Hello World");
	assert(!ps.empty());
	assert(ps.length() == 11);
	mylog("  copy: %s\n", ps.c_str());
	
	// copy 指定长度
	ps.copy("Hello World", 5);
	mylog("  copy(5): %s\n", ps.c_str());
	assert(ps.length() == 5);
	
	// clear
	ps.clear();
	assert(ps.empty());
	
	// append
	ps.append("Test");
	ps.append(" String");
	mylog("  append: %s\n", ps.c_str());
	
	// append 字符
	ps.append('!');
	mylog("  append char: %s\n", ps.c_str());
	
	// substract (删除末尾字符)
	ps.copy("abcdef");
	ps.substract(2);
	mylog("  substract(2): %s\n", ps.c_str());
	assert(ps.length() == 4);
	
	mylog("[V]Basic operations passed\n");
}

void test_pstring_string_operations()
{
	mylog("Testing pstring string operations...\n");
	
	char buffer1[100], buffer2[100], buffer3[100];
	memset(buffer1, 0, sizeof(buffer1));
	memset(buffer2, 0, sizeof(buffer2));
	memset(buffer3, 0, sizeof(buffer3));
	
	pstring ps1(buffer1, 100);
	pstring ps2(buffer2, 100);
	pstring ps3(buffer3, 100);
	
	ps1.copy("Hello World");
	ps2.copy("hello world");
	ps3.copy("Test String");
	
	// 比较操作
	assert(ps1.compare("Hello World") == 0);
	assert(ps1.compare("hello world", true) == 0);  // 忽略大小写
	mylog("  compare: %d\n", ps1.compare(ps2.c_str()));
	
	// 查找操作
	int pos = ps1.find('o');
	mylog("  find 'o': position %d\n", pos);
	assert(pos == 4);
	
	pos = ps1.find("World");
	mylog("  find 'World': position %d\n", pos);
	assert(pos == 6);
	
	pos = ps1.find_last_of('o');
	mylog("  find_last_of 'o': position %d\n", pos);
	assert(pos == 7);
	
	// contains
	assert(ps1.contains('H'));
	assert(ps1.contains("World"));
	assert(!ps1.contains("xyz"));
	
	// start_with 和 end_with
	assert(ps1.start_with("Hello", false));
	assert(ps1.start_with("hello", true));  // 忽略大小写
	assert(ps1.end_with("World", false));
	assert(ps1.end_with("world", true));  // 忽略大小写
	
	// replace
	ps1.copy("Hello World Hello");
	ps1.replace("Hello", "Hi");
	mylog("  replace first: %s\n", ps1.c_str());
	
	ps1.copy("Hello World Hello");
	int count = ps1.replace_all("Hello", "Hi");
	mylog("  replace_all (count=%d): %s\n", count, ps1.c_str());
	
	// insert
	ps1.copy("Hello World");
	ps1.insert(6, "Beautiful ");
	mylog("  insert: %s\n", ps1.c_str());
	
	ps1.insert(0, 'X');
	mylog("  insert char: %s\n", ps1.c_str());
	
	// erase
	ps1.copy("Hello World");
	ps1.erase(5, 1);  // 删除空格
	mylog("  erase: %s\n", ps1.c_str());
	
	// substr
	char sub[32];
	ps1.copy("Hello World");
	ps1.substr(6, 5, sub, 32);
	mylog("  substr(6,5): %s\n", sub);
	
	mylog("[V]String operations passed\n");
}

void test_pstring_conversions()
{
	mylog("Testing pstring conversions and formatting...\n");
	
	char buffer[100];
	memset(buffer, 0, sizeof(buffer));
	
	pstring ps(buffer, 100);
	
	// 数值转换 (from_xxx)
	ps.from_int(12345);
	mylog("  from_int: %s\n", ps.c_str());
	
	ps.from_double(123.456);
	mylog("  from_double: %s\n", ps.c_str());
	
	ps.from_int64(9876543210LL);
	mylog("  from_int64: %s\n", ps.c_str());
	
	// 数值解析 (to_xxx)
	ps.copy("42");
	int val = ps.to_int(0, 10);
	mylog("  to_int: %d\n", val);
	assert(val == 42);
	
	ps.copy("3.14159");
	double dval = ps.to_double(0.0);
	mylog("  to_double: %f\n", dval);
	
	ps.copy("FF");
	uint hex_val = ps.to_hex(0);
	mylog("  to_hex: %u\n", hex_val);
	assert(hex_val == 255);
	
	ps.copy("true");
	bool bval = ps.to_bool(false);
	mylog("  to_bool: %d\n", bval);
	assert(bval == true);
	
	// 格式化
	ps.format("Number: %d, Float: %.2f", 42, 3.14);
	mylog("  format: %s\n", ps.c_str());
	
	ps.format_append(", String: %s", "test");
	mylog("  format_append: %s\n", ps.c_str());
	
	// 大小写转换
	ps.copy("Hello World");
	ps.toupper();
	mylog("  toupper: %s\n", ps.c_str());
	
	ps.tolower();
	mylog("  tolower: %s\n", ps.c_str());
	
	// 单个字符大小写转换
	ps.copy("hello");
	ps.toupper(0);
	mylog("  toupper(0): %s\n", ps.c_str());
	
	// 去空白字符
	ps.copy("  Hello World  ");
	int trimmed = ps.trim();
	mylog("  trim (removed %d chars): '%s'\n", trimmed, ps.c_str());
	
	ps.copy("  Hello World  ");
	ps.trim_left();
	mylog("  trim_left: '%s'\n", ps.c_str());
	
	ps.copy("  Hello World  ");
	ps.trim_right();
	mylog("  trim_right: '%s'\n", ps.c_str());
	
	mylog("[V]Conversions and formatting passed\n");
}

void test_pstring_operators()
{
	mylog("Testing pstring operators...\n");
	
	char buffer1[100], buffer2[100], buffer3[100];
	memset(buffer1, 0, sizeof(buffer1));
	memset(buffer2, 0, sizeof(buffer2));
	memset(buffer3, 0, sizeof(buffer3));
	
	pstring ps1(buffer1, 100);
	pstring ps2(buffer2, 100);
	pstring ps3(buffer3, 100);
	
	// 赋值操作符
	ps1 = "Hello";
	ps2 = ps1;
	ps3 = 42;
	mylog("  assignment: ps1='%s', ps2='%s', ps3='%s'\n", ps1.c_str(), ps2.c_str(), ps3.c_str());
	
	// += 操作符
	ps1 += " World";
	ps1 += '!';
	ps1 += ps2;
	mylog("  += operator: %s\n", ps1.c_str());
	
	// 比较操作符
	ps1.copy("ABC");
	ps2.copy("ABC");
	ps3.copy("XYZ");
	
	assert(ps1 == ps2);
	assert(ps1 != ps3);
	assert(ps1 < ps3);
	assert(ps3 > ps1);
	assert(ps1 == "ABC");
	assert(ps1 != "XYZ");
	
	// [] 操作符
	ps1.copy("Hello");
	assert(ps1[0] == 'H');
	assert(ps1[4] == 'o');
	ps1[0] = 'h';
	mylog("  [] operator: %s\n", ps1.c_str());
	
	mylog("[V]Operators passed\n");
}

void test_pwstring_basic_operations()
{
	mylog("Testing pwstring basic operations...\n");
	
	// 创建宽字符缓冲区
	wchar_t wbuffer[100];
	wmemset(wbuffer, 0, sizeof(wbuffer)/sizeof(wchar_t));
	
	// 使用新的构造函数初始化
	pwstring pws(wbuffer, 100);
	
	// 基本属性测试
	assert(pws.empty());
	assert(pws.length() == 0);
	assert(pws.capacity() == 99);
	assert(pws.max_size() == 100);
	
	// copy 方法
	pws.copy(L"Hello World");
	assert(!pws.empty());
	assert(pws.length() == 11);
	mywlog(L"  copy: %ls\n", pws.c_str());
	
	// copy 指定长度
	pws.copy(L"Hello World", 5);
	mywlog(L"  copy(5): %ls\n", pws.c_str());
	assert(pws.length() == 5);
	
	// clear
	pws.clear();
	assert(pws.empty());
	
	// append
	pws.append(L"Test");
	pws.append(L" String");
	mywlog(L"  append: %ls\n", pws.c_str());
	
	// append 字符
	pws.append(L'!');
	mywlog(L"  append char: %ls\n", pws.c_str());
	
	// substract
	pws.copy(L"abcdef");
	pws.substract(2);
	mywlog(L"  substract(2): %ls\n", pws.c_str());
	assert(pws.length() == 4);
	
	mylog("[V]Basic operations passed\n");
}

void test_pwstring_string_operations()
{
	mylog("Testing pwstring string operations...\n");
	
	wchar_t wbuffer1[100], wbuffer2[100], wbuffer3[100];
	wmemset(wbuffer1, 0, sizeof(wbuffer1)/sizeof(wchar_t));
	wmemset(wbuffer2, 0, sizeof(wbuffer2)/sizeof(wchar_t));
	wmemset(wbuffer3, 0, sizeof(wbuffer3)/sizeof(wchar_t));
	
	pwstring pws1(wbuffer1, 100);
	pwstring pws2(wbuffer2, 100);
	pwstring pws3(wbuffer3, 100);
	
	pws1.copy(L"Hello World");
	pws2.copy(L"hello world");
	pws3.copy(L"Test String");
	
	// 比较操作
	assert(pws1.compare(L"Hello World") == 0);
	assert(pws1.compare(L"hello world", true) == 0);  // 忽略大小写
	mylog("  compare result: %d\n", pws1.compare(pws2.c_str()));
	
	// 查找操作
	int pos = pws1.find(L'o');
	mylog("  find 'o': position %d\n", pos);
	assert(pos == 4);
	
	pos = pws1.find(L"World");
	mylog("  find 'World': position %d\n", pos);
	assert(pos == 6);
	
	pos = pws1.find_last_of(L'o');
	mylog("  find_last_of 'o': position %d\n", pos);
	assert(pos == 7);
	
	// contains
	assert(pws1.contains(L'H'));
	assert(pws1.contains(L"World"));
	assert(!pws1.contains(L"xyz"));
	
	// start_with 和 end_with
	assert(pws1.start_with(L"Hello", false));
	assert(pws1.start_with(L"hello", true));  // 忽略大小写
	assert(pws1.end_with(L"World", false));
	assert(pws1.end_with(L"world", true));  // 忽略大小写
	
	// replace
	pws1.copy(L"Hello World Hello");
	pws1.replace(L"Hello", L"Hi");
	mywlog(L"  replace first: %ls\n", pws1.c_str());
	
	pws1.copy(L"Hello World Hello");
	int count = pws1.replace_all(L"Hello", L"Hi");
	mywlog(L"  replace_all (count=%d): %ls\n", count, pws1.c_str());
	
	// insert
	pws1.copy(L"Hello World");
	pws1.insert(6, L"Beautiful ");
	mywlog(L"  insert: %ls\n", pws1.c_str());
	
	pws1.insert(0, L'X');
	mywlog(L"  insert char: %ls\n", pws1.c_str());
	
	// erase
	pws1.copy(L"Hello World");
	pws1.erase(5, 1);  // 删除空格
	mywlog(L"  erase: %ls\n", pws1.c_str());
	
	// substr
	wchar_t sub[32];
	pws1.copy(L"Hello World");
	pws1.substr(6, 5, sub, 32);
	mywlog(L"  substr(6,5): %ls\n", sub);
	
	mylog("[V]String operations passed\n");
}

void test_pwstring_conversions()
{
	mylog("Testing pwstring conversions and formatting...\n");
	
	wchar_t wbuffer[100];
	wmemset(wbuffer, 0, sizeof(wbuffer)/sizeof(wchar_t));
	
	pwstring pws(wbuffer, 100);
	
	// 数值转换 (from_xxx)
	pws.from_int(12345);
	mywlog(L"  from_int: %ls\n", pws.c_str());
	
	pws.from_double(123.456);
	mywlog(L"  from_double: %ls\n", pws.c_str());
	
	pws.from_int64(9876543210LL);
	mywlog(L"  from_int64: %ls\n", pws.c_str());
	
	// 数值解析 (to_xxx)
	pws.copy(L"42");
	int val = pws.to_int(0, 10);
	mylog("  to_int: %d\n", val);
	assert(val == 42);
	
	pws.copy(L"3.14159");
	double dval = pws.to_double(0.0);
	mylog("  to_double: %f\n", dval);
	
	pws.copy(L"FF");
	uint hex_val = pws.to_hex(0);
	mylog("  to_hex: %u\n", hex_val);
	assert(hex_val == 255);
	
	pws.copy(L"true");
	bool bval = pws.to_bool(false);
	mylog("  to_bool: %d\n", bval);
	assert(bval == true);
	
	// 格式化
	pws.format(L"Number: %d, Float: %.2f", 42, 3.14);
	mywlog(L"  format: %ls\n", pws.c_str());
	
	pws.format_append(L", String: %ls", L"test");
	mywlog(L"  format_append: %ls\n", pws.c_str());
	
	// 大小写转换
	pws.copy(L"Hello World");
	pws.toupper();
	mywlog(L"  toupper: %ls\n", pws.c_str());
	
	pws.tolower();
	mywlog(L"  tolower: %ls\n", pws.c_str());
	
	// 单个字符大小写转换
	pws.copy(L"hello");
	pws.toupper(0);
	mywlog(L"  toupper(0): %ls\n", pws.c_str());
	
	// 去空白字符
	pws.copy(L"  Hello World  ");
	int trimmed = pws.trim();
	mywlog(L"  trim (removed %d chars): '%ls'\n", trimmed, pws.c_str());
	
	pws.copy(L"  Hello World  ");
	pws.trim_left();
	mywlog(L"  trim_left: '%ls'\n", pws.c_str());
	
	pws.copy(L"  Hello World  ");
	pws.trim_right();
	mywlog(L"  trim_right: '%ls'\n", pws.c_str());
	
	mylog("[V]Conversions and formatting passed\n");
}

void test_pwstring_operators()
{
	mylog("Testing pwstring operators...\n");
	
	wchar_t wbuffer1[100], wbuffer2[100], wbuffer3[100];
	wmemset(wbuffer1, 0, sizeof(wbuffer1)/sizeof(wchar_t));
	wmemset(wbuffer2, 0, sizeof(wbuffer2)/sizeof(wchar_t));
	wmemset(wbuffer3, 0, sizeof(wbuffer3)/sizeof(wchar_t));
	
	pwstring pws1(wbuffer1, 100);
	pwstring pws2(wbuffer2, 100);
	pwstring pws3(wbuffer3, 100);
	
	// 赋值操作符
	pws1 = L"Hello";
	pws2 = pws1;
	pws3 = 42;
	mywlog(L"  assignment: pws1='%ls', pws2='%ls', pws3='%ls'\n", pws1.c_str(), pws2.c_str(), pws3.c_str());
	
	// += 操作符
	pws1 += L" World";
	pws1 += L'!';
	pws1 += pws2;
	mywlog(L"  += operator: %ls\n", pws1.c_str());
	
	// 比较操作符
	pws1.copy(L"ABC");
	pws2.copy(L"ABC");
	pws3.copy(L"XYZ");
	
	assert(pws1 == pws2);
	assert(pws1 != pws3);
	assert(pws1 < pws3);
	assert(pws3 > pws1);
	assert(pws1 == L"ABC");
	assert(pws1 != L"XYZ");
	
	// [] 操作符
	pws1.copy(L"Hello");
	assert(pws1[0] == L'H');
	assert(pws1[4] == L'o');
	pws1[0] = L'h';
	mywlog(L"  [] operator: %ls\n", pws1.c_str());
	
	mylog("[V]Operators passed\n");
}

// 测试 string32/vstring 转换为 pstring
void test_string_conversion_pstring()
{
	mylog("Testing string to pstring conversion:\n");
	
	// 测试 string32 转换为 pstring
	string32 s32("Hello World");
	mylog("  Original string32: '%s'\n", s32.c_str());
	
	// 转换为 pstring (非 const 版本)
	pstring ps = s32.pstring();
	mylog("  Converted to pstring: '%s'\n", ps.c_str());
	assert(strcmp(ps.c_str(), "Hello World") == 0);
	
	// 通过 pstring 修改原始字符串
	ps.append(" - Modified");
	mylog("  After pstring modification: '%s'\n", s32.c_str());
	mylog("  pstring content: '%s'\n", ps.c_str());
	assert(strcmp(s32.c_str(), "Hello World - Modified") == 0);
	
	// 测试 const 版本转换
	const string32 const_s32("Const String");
	const pstring const_ps = const_s32.pstring();
	mylog("  Const string32 to const pstring: '%s'\n", const_ps.c_str());
	assert(strcmp(const_ps.c_str(), "Const String") == 0);
	
	// 测试 vstring 转换为 pstring
	vstring vs("Variable String");
	mylog("  Original vstring: '%s'\n", vs.c_str());
	
	pstring ps_from_vs = vs.pstring();
	mylog("  vstring to pstring: '%s'\n", ps_from_vs.c_str());
	assert(strcmp(ps_from_vs.c_str(), "Variable String") == 0);
	
	// 通过 pstring 修改 vstring, 这里无法append，因为 pstring 指向的是 vstring 的short
	ps_from_vs.append(" - Extended");
	mylog("  After pstring modification of vstring: '%s'\n", vs.c_str());
	assert(strcmp(vs.c_str(), "Variable String") == 0);
	
	// 测试内存共享 - 确认修改是双向的
	string32 s32_share("Shared");
	pstring ps_share = s32_share.pstring();
	
	// 通过原字符串修改
	s32_share.append(" Data");
	mylog("  After string32 modification: pstring='%s', string32='%s'\n", 
		   ps_share.c_str(), s32_share.c_str());
	assert(strcmp(ps_share.c_str(), "Shared Data") == 0);
	
	// 通过 pstring 修改
	ps_share.append(" Modified");
	mylog("  After pstring modification: pstring='%s', string32='%s'\n", 
		   ps_share.c_str(), s32_share.c_str());
	assert(strcmp(s32_share.c_str(), "Shared Data Modified") == 0);
	
	mylog("  OK String to pstring conversion passed\n");
}

// 测试 wstring32/vwstring 转换为 pwstring
void test_wstring_conversion_to_pwstring()
{
	mylog("Testing wstring to pwstring conversion:\n");
	
	// 测试 wstring32 转换为 pwstring
	wstring32 ws32(L"Hello World");
	mywlog(L"  Original wstring32: '%ls'\n", ws32.c_str());
	
	// 转换为 pwstring (非 const 版本)
	pwstring pws = ws32.pstring();
	mywlog(L"  Converted to pwstring: '%ls'\n", pws.c_str());
	assert(wcscmp(pws.c_str(), L"Hello World") == 0);
	
	// 通过 pwstring 修改原始字符串
	pws.append(L" - Modified");
	mywlog(L"  After pwstring modification: '%ls'\n", ws32.c_str());
	mywlog(L"  pwstring content: '%ls'\n", pws.c_str());
	assert(wcscmp(ws32.c_str(), L"Hello World - Modified") == 0);
	
	// 测试 const 版本转换
	const wstring32 const_ws32(L"Const String");
	const pwstring const_pws = const_ws32.pstring();
	mywlog(L"  Const wstring32 to const pwstring: '%ls'\n", const_pws.c_str());
	assert(wcscmp(const_pws.c_str(), L"Const String") == 0);
	
	// 测试 vwstring 转换为 pwstring
	vwstring vws(L"Variable String");
	mywlog(L"  Original vwstring: '%ls'\n", vws.c_str());
	
	pwstring pws_from_vws = vws.pstring();
	mywlog(L"  vwstring to pwstring: '%ls'\n", pws_from_vws.c_str());
	assert(wcscmp(pws_from_vws.c_str(), L"Variable String") == 0);
	
	// 通过 pwstring 修改 vwstring，这里无法append，因为 pstring 指向的是 vstring 的short
	pws_from_vws.append(L" - Extended");
	mywlog(L"  After pwstring modification of vwstring: '%ls'\n", vws.c_str());
	assert(wcscmp(vws.c_str(), L"Variable String") == 0);
	
	// 测试内存共享 - 确认修改是双向的
	wstring32 ws32_share(L"Shared");
	pwstring pws_share = ws32_share.pstring();
	
	// 通过原字符串修改
	ws32_share.append(L" Data");
	mywlog(L"  After wstring32 modification: pwstring='%ls', wstring32='%ls'\n", 
			pws_share.c_str(), ws32_share.c_str());
	assert(wcscmp(pws_share.c_str(), L"Shared Data") == 0);
	
	// 通过 pwstring 修改
	pws_share.append(L" Modified");
	mywlog(L"  After pwstring modification: pwstring='%ls', wstring32='%ls'\n", 
			pws_share.c_str(), ws32_share.c_str());
	assert(wcscmp(ws32_share.c_str(), L"Shared Data Modified") == 0);
	
	mylog("  OK Wstring to pwstring conversion passed\n");
}

// 测试编码转换接口
void test_encoding_conversion_interfaces() {
	mylog("Testing encoding conversion interfaces...\n");
	
	// 测试 UTF-8 转换
	{
		mylog("  Testing UTF-8 conversion:\n");
		wstring32 ws;
		const char* utf8_text = "Hello World UTF8";
		
		ws.from_utf8(utf8_text);
		mywlog(L"    from_utf8: '%ls'\n", ws.c_str());
		assert(ws.length() > 0);
		
		char utf8_buffer[64];
		ws.to_utf8(utf8_buffer, sizeof(utf8_buffer));
		mylog("    to_utf8: '%s'\n", utf8_buffer);
		
		// 验证双向转换
		wstring32 ws2;
		ws2.from_utf8(utf8_buffer);
		assert(wcscmp(ws.c_str(), ws2.c_str()) == 0);
		mylog("    UTF-8 round-trip conversion OK\n");
	}
	
	// 测试 GBK 转换  
	{
		mylog("  Testing GBK conversion:\n");
		wstring32 ws;
		const char* gbk_text = "Test GBK Text";
		
		ws.from_gbk(gbk_text);
		mywlog(L"    from_gbk: '%ls'\n", ws.c_str());
		assert(ws.length() > 0);
		
		char gbk_buffer[64];
		ws.to_gbk(gbk_buffer, sizeof(gbk_buffer));
		mylog("    to_gbk: '%s'\n", gbk_buffer);
		mylog("    GBK conversion OK\n");
	}
	
	// 测试 ANSI 转换
	{
		mylog("  Testing ANSI conversion:\n");
		wstring32 ws;
		const char* ansi_text = "Test ANSI Text";
		
		ws.from_ansi(ansi_text);
		mywlog(L"    from_ansi: '%ls'\n", ws.c_str());
		assert(ws.length() > 0);
		
		char ansi_buffer[64];
		ws.to_ansi(ansi_buffer, sizeof(ansi_buffer));
		mylog("    to_ansi: '%s'\n", ansi_buffer);
		mylog("    ANSI conversion OK\n");
	}
	
	// 测试 pwstring 的编码转换
	{
		mylog("  Testing pwstring encoding conversion:\n");
		wchar_t buffer[64];
		pwstring pws(buffer, 64);
		
		const char* test_text = "PWString UTF8 Test";
		pws.from_utf8(test_text);
		mywlog(L"    pwstring from_utf8: '%ls'\n", pws.c_str());
		assert(pws.length() > 0);
		
		char out_buffer[64];
		pws.to_utf8(out_buffer, sizeof(out_buffer));
		mylog("    pwstring to_utf8: '%s'\n", out_buffer);
		mylog("    PWString encoding conversion OK\n");
	}
	
	// 测试 vwstring 的编码转换
	{
		mylog("  Testing vwstring encoding conversion:\n");
		vwstring vws;
		
		const char* long_utf8 = "This is a longer UTF-8 string that should trigger vwstring expansion";
		vws.from_utf8(long_utf8);
		mywlog(L"    vwstring from_utf8: '%ls' (length: %d)\n", vws.c_str(), vws.length());
		assert(vws.length() > 16); // 应该触发扩容
		
		char long_buffer[128];
		vws.to_utf8(long_buffer, sizeof(long_buffer));
		mylog("    vwstring to_utf8: '%s'\n", long_buffer);
		mylog("    VWString encoding conversion with expansion OK\n");
	}
	
	mylog("  All encoding conversion tests passed!\n");
}

// 测试 scanf 和 reserve 接口
void test_scanf_and_reserve_interfaces() {
	mylog("Testing scanf and reserve interfaces...\n");
	
	// 测试 string32 的 scanf 功能
	{
		mylog("  Testing string32 scanf:\n");
		string32 s32("123 456 789.5 hello");
		
		int a, b;
		float c;
		char word[32];
		
		int parsed = s32.scanf("%d %d %f %s", &a, &b, &c, word);
		mylog("    Input: '%s'\n", s32.c_str());
		mylog("    Parsed %d items: a=%d, b=%d, c=%.1f, word='%s'\n", parsed, a, b, c, word);
		assert(parsed == 4);
		assert(a == 123);
		assert(b == 456);
		assert(c > 789.4f && c < 789.6f);  // 浮点数比较
		assert(strcmp(word, "hello") == 0);
	}
	
	// 测试 wstring32 的 scanf 功能
	{
		mylog("  Testing wstring32 scanf:\n");
		wstring32 ws32(L"999 888");
		
		int x, y;
		int parsed = ws32.scanf(L"%d %d", &x, &y);
		mywlog(L"    Input: '%ls'\n", ws32.c_str());
		mylog("    Parsed %d items: x=%d, y=%d\n", parsed, x, y);
		
#ifdef SCL_WIN
		// Windows 平台才有完整的 scanf 实现
		assert(parsed == 2);
		assert(x == 999);
		assert(y == 888);
		mylog("    WString scanf OK\n");
#else
		mylog("    WString scanf not fully implemented on non-Windows platforms\n");
#endif
	}
	
	// 测试 reserve 功能
	{
		mylog("  Testing reserve functionality:\n");
		
		// 测试 vstring 的 reserve
		vstring vs("Short");
		mylog("    vstring initial capacity: %d, length: %d\n", vs.capacity(), vs.length());
		
		// 保留更大的容量
		vs.reserve(100);
		mylog("    After reserve(100): capacity=%d, length=%d\n", vs.capacity(), vs.length());
		assert(vs.capacity() >= 100);
		assert(strcmp(vs.c_str(), "Short") == 0);  // 内容不变
		
		// 添加长字符串测试扩容
		vs.append(" - This is a much longer string that should fit in the reserved space");
		mylog("    After append long string: capacity=%d, length=%d\n", vs.capacity(), vs.length());
		mylog("    Content: '%s'\n", vs.c_str());
		assert(vs.length() > 50);  // 应该是长字符串了
		
		// 测试 vwstring 的 reserve
		vwstring vws(L"Wide");
		mylog("    vwstring initial capacity: %d, length: %d\n", vws.capacity(), vws.length());
		
		vws.reserve(200);
		mylog("    After reserve(200): capacity=%d, length=%d\n", vws.capacity(), vws.length());
		assert(vws.capacity() >= 200);
		assert(wcscmp(vws.c_str(), L"Wide") == 0);  // 内容不变
	}
	
	// 测试固定字符串的 reserve（应该无效果）
	{
		mylog("  Testing reserve on fixed strings:\n");
		string32 s32("Fixed");
		int old_capacity = s32.capacity();
		s32.reserve(1000);  // 对固定容量字符串无效
		mylog("    string32 capacity before: %d, after reserve(1000): %d\n", old_capacity, s32.capacity());
		assert(s32.capacity() == old_capacity);  // 容量应该不变
		
		wstring32 ws32(L"WFixed");
		int old_wcapacity = ws32.capacity();
		ws32.reserve(1000);  // 对固定容量字符串无效
		mylog("    wstring32 capacity before: %d, after reserve(1000): %d\n", old_wcapacity, ws32.capacity());
		assert(ws32.capacity() == old_wcapacity);  // 容量应该不变
	}
	
	// 测试复杂的 scanf 格式
	{
		mylog("  Testing complex scanf patterns:\n");
		string32 s32("Name:John Age:25 Score:95.5");
		
		char name[32];
		int age;
		float score;
		
		int parsed = s32.scanf("Name:%s Age:%d Score:%f", name, &age, &score);
		mylog("    Complex pattern input: '%s'\n", s32.c_str());
		mylog("    Parsed %d items: name='%s', age=%d, score=%.1f\n", parsed, name, age, score);
		assert(parsed == 3);
		assert(strcmp(name, "John") == 0);
		assert(age == 25);
		assert(score > 95.4f && score < 95.6f);
	}
	
	mylog("All scanf and reserve tests passed!\n");
}


void testString2025()
{
	mylog("=== Testing string32 (char-based) ===\n");
	test_string32_basic_operations();
	test_string32_string_operations();
	test_string32_conversions();
	test_string32_operators();
	
	mylog("\n=== Testing wstring32 (wchar_t-based) ===\n");
	test_wstring32_basic_operations();
	test_wstring32_string_operations();
	test_wstring32_conversions();
	test_wstring32_operators();
	
	mylog("\n=== Testing pstring (pointer-based char string) ===\n");
	test_pstring_basic_operations();
	test_pstring_string_operations();
	test_pstring_conversions();
	test_pstring_operators();
	
	mylog("\n=== Testing pwstring (pointer-based wchar string) ===\n");
	test_pwstring_basic_operations();
	test_pwstring_string_operations();
	test_pwstring_conversions();
	test_pwstring_operators();
	
	mylog("\n=== Testing string conversion methods ===\n");
	test_string_conversion_pstring();
	test_wstring_conversion_to_pwstring();
	
	mylog("\n=== Testing encoding conversion interfaces ===\n");
	test_encoding_conversion_interfaces();
	
	mylog("\n=== Testing scanf and reserve interfaces ===\n");
	test_scanf_and_reserve_interfaces();
	
	mylog("\n=== All tests completed successfully! ===\n");
}

