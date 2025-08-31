
#include "testIniFile.h"

#include "scl/ini_file.h"
#include "scl/string.h"
#include "scl/encoding.h"

#include <stdio.h>

using scl::ini_parser;
using scl::string;
using scl::wstring;

void testIniFile()
{
	ini_parser ini;
	ini.open("a.ini", "rb");
	string32 netIp;
	ini.get_string("db", "ip", netIp.c_str(), netIp.capacity());
	assert(netIp == "127.0.0.1");
	string64 s;
	ini.get_string(_ANSI("窗口布局"), _ANSI("高度"), s.c_str(), s.max_sizeof());
	assert(s == "472");

	const int position	= ini.get_int(_ANSI("窗口布局"), _ANSI("位置"));
	const int left		= ini.get_int(_ANSI("窗口布局"), _ANSI("左边"));
	const int top		= ini.get_int(_ANSI("窗口布局"), _ANSI("顶边"));
	const int width		= ini.get_int(_ANSI("窗口布局"), _ANSI("宽度"));
	const int height	= ini.get_int(_ANSI("窗口布局"), _ANSI("高度"));

	const int global_a	= ini.get_int("", "global_a");
	const int global_b	= ini.get_int("", "global_b");

	const int fake = ini.get_int(_ANSI("窗口布局"), "fake");

	string<128> title;
	ini.get_string(_ANSI("窗口布局"), "title", title.c_str(), title.max_sizeof());

	string<256> content1;
	ini.get_string(_ANSI("窗口布局"), "content1", content1.c_str(), content1.max_sizeof());

	pstring content2;
	content2.init(new char[1024 * 1024], 1024 * 1024);
	content2.clear();
	ini.get_string(_ANSI("窗口布局"), "content2", content2.c_str(), content2.max_sizeof());

	assert(position == 0);
	assert(left == 265);
	assert(top == 291);
	assert(width == 889);
	assert(height == 472);
	assert(global_a == 16);
	assert(global_b == 36);
	assert(fake == 0);
	wstring128 wtitle;
	wtitle.from_ansi(title.c_str());
	assert(wtitle == L"我们是player@mail.com");
	wstring128 wcontent1;
	wcontent1.from_ansi(content1.c_str());
	assert(wcontent1 == L"\n \t我a!@#$%^&*\n()@@_+}{\":<>?\\|\"\"");

	delete content2.c_str();

	printf("test ini file \t\tOK!\n");
}

void testIniWriter()
{
	const char* test_file = "test_writer.ini";
	
	// Write test data using ini_writer
	{
		scl::ini_writer writer(test_file);
		assert(writer.open());
		
		// Write global section values
		writer.write("global_int", 42);
		writer.write("global_uint", 123u);
		writer.write("global_float", 3.14159f);
		writer.write("global_int64", 9223372036854775807LL);
		writer.write("global_uint64", 18446744073709551615ULL);
		writer.write("global_bool_true", true);
		writer.write("global_bool_false", false);
		
		// Write a section
		writer.write_section("TestSection");
		writer.write("section_int", -999);
		writer.write("section_uint", 456u);
		writer.write("section_float", -2.718f);
		writer.write("section_int64", -1234567890123456789LL);
		writer.write("section_uint64", 9876543210987654321ULL);
		writer.write("section_bool_true", true);
		writer.write("section_bool_false", false);
		writer.write("section_string", "Hello World");
		
		// Write another section
		writer.write_section("Numbers");
		writer.write("zero", 0);
		writer.write("negative", -1);
		writer.write("positive", 1);
		
		writer.close();
	}
	
	// Read and verify the data using ini_parser
	{
		scl::ini_parser reader;
		assert(reader.open(test_file, "rb"));
		
		// Verify global section values
		assert(reader.get_int("", "global_int") == 42);
		assert(reader.get_uint("", "global_uint") == 123u);
		assert(reader.get_float("", "global_float") - 3.14159f < 0.001f);
		assert(reader.get_int64("", "global_int64") == 9223372036854775807LL);
		assert(reader.get_uint64("", "global_uint64") == 18446744073709551615ULL);
		assert(reader.get_bool("", "global_bool_true") == true);
		assert(reader.get_bool("", "global_bool_false") == false);
		
		// Verify TestSection values
		assert(reader.get_int("TestSection", "section_int") == -999);
		assert(reader.get_uint("TestSection", "section_uint") == 456u);
		assert(reader.get_float("TestSection", "section_float") + 2.718f < 0.001f);
		assert(reader.get_int64("TestSection", "section_int64") == -1234567890123456789LL);
		assert(reader.get_uint64("TestSection", "section_uint64") == 9876543210987654321ULL);
		assert(reader.get_bool("TestSection", "section_bool_true") == true);
		assert(reader.get_bool("TestSection", "section_bool_false") == false);
		
		scl::string<32> str;
		reader.get_string("TestSection", "section_string", str.c_str(), str.max_sizeof());
		assert(str == "Hello World");
		
		// Verify Numbers section
		assert(reader.get_int("Numbers", "zero") == 0);
		assert(reader.get_int("Numbers", "negative") == -1);
		assert(reader.get_int("Numbers", "positive") == 1);
	}
	
	// Clean up test file
	remove(test_file);
	
	printf("test ini writer \tOK!\n");
}

