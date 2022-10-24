
#include "testIniFile.h"

#include "scl/ini_file.h"
#include "scl/string.h"
#include "scl/encoding.h"

#include <stdio.h>

using scl::ini_file;
using scl::string;
using scl::wstring;

void testIniFile()
{
	ini_file ini;
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

