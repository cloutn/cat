
#include "testIniFile.h"

#include "scl/ini_file.h"
#include "scl/string.h"
#include "scl/encoding.h"
#include "scl/vector.h"

#include <stdio.h>

using scl::ini_parser;
using scl::string;
using scl::wstring;
using scl::vstring;
using scl::wstring128;

void testIniFile()
{
	ini_parser ini;
	ini.open("a.ini", "rb");
	vstring netIp = ini.get_string("db", "ip");
	assert(netIp == "127.0.0.1");
	vstring s = ini.get_string(_ANSI("窗口布局"), _ANSI("高度"));
	assert(s == "472");

	const int position	= ini.get_int(_ANSI("窗口布局"), _ANSI("位置"));
	const int left		= ini.get_int(_ANSI("窗口布局"), _ANSI("左边"));
	const int top		= ini.get_int(_ANSI("窗口布局"), _ANSI("顶边"));
	const int width		= ini.get_int(_ANSI("窗口布局"), _ANSI("宽度"));
	const int height	= ini.get_int(_ANSI("窗口布局"), _ANSI("高度"));

	const int global_a	= ini.get_int("", "global_a");
	const int global_b	= ini.get_int("", "global_b");

	const int fake = ini.get_int(_ANSI("窗口布局"), "fake");

	vstring title = ini.get_string(_ANSI("窗口布局"), "title");

	vstring content1 = ini.get_string(_ANSI("窗口布局"), "content1");

	vstring content2 = ini.get_string(_ANSI("窗口布局"), "content2");

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
		
		// Write vector tests
		writer.write_section("VectorTests");
		
		// Test scl::vector3
		scl::vector3 test_vector3;
		test_vector3.set(1.5f, 2.5f, 3.5f);
		writer.write("test_vector3", test_vector3);
		
		scl::vector3 zero_vector3;
		zero_vector3.clear();
		writer.write("zero_vector3", zero_vector3);
		
		scl::vector3 negative_vector3;
		negative_vector3.set(-1.2f, -2.3f, -3.4f);
		writer.write("negative_vector3", negative_vector3);
		
		// Test scl::vector2i
		scl::vector2i test_vector2i;
		test_vector2i.set(10, 20);
		writer.write("test_vector2i", test_vector2i);
		
		scl::vector2i zero_vector2i;
		zero_vector2i.clear();
		writer.write("zero_vector2i", zero_vector2i);
		
		scl::vector2i negative_vector2i;
		negative_vector2i.set(-5, -10);
		writer.write("negative_vector2i", negative_vector2i);
		
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
		
		vstring str = reader.get_string("TestSection", "section_string");
		assert(str == "Hello World");
		
		// Verify Numbers section
		assert(reader.get_int("Numbers", "zero") == 0);
		assert(reader.get_int("Numbers", "negative") == -1);
		assert(reader.get_int("Numbers", "positive") == 1);
		
		// Verify vector tests
		// Test scl::vector3
		scl::vector3 default_vector3;
		default_vector3.clear();
		
		scl::vector3 read_vector3 = reader.get<scl::vector3>("VectorTests", "test_vector3", default_vector3);
		assert(read_vector3.equal(1.5f, 2.5f, 3.5f));
		
		scl::vector3 read_zero_vector3 = reader.get<scl::vector3>("VectorTests", "zero_vector3", default_vector3);
		assert(read_zero_vector3.equal(0.0f, 0.0f, 0.0f));
		
		scl::vector3 read_negative_vector3 = reader.get<scl::vector3>("VectorTests", "negative_vector3", default_vector3);
		assert(read_negative_vector3.equal(-1.2f, -2.3f, -3.4f));
		
		// Test scl::vector2i
		scl::vector2i default_vector2i;
		default_vector2i.clear();
		
		scl::vector2i read_vector2i = reader.get<scl::vector2i>("VectorTests", "test_vector2i", default_vector2i);
		assert(read_vector2i.is(10, 20));
		
		scl::vector2i read_zero_vector2i = reader.get<scl::vector2i>("VectorTests", "zero_vector2i", default_vector2i);
		assert(read_zero_vector2i.is(0, 0));
		
		scl::vector2i read_negative_vector2i = reader.get<scl::vector2i>("VectorTests", "negative_vector2i", default_vector2i);
		assert(read_negative_vector2i.is(-5, -10));
	}
	
	// Clean up test file
	remove(test_file);
	
	printf("test ini writer \tOK!\n");
	printf("test vector3 & vector2i \tOK!\n");
}

