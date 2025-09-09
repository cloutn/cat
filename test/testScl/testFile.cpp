
#include "testFile.h"

#include "scl/file.h"
#include "scl/directory.h"
#include "scl/encoding.h"
#include "scl/file_reader.h"
#include "scl/assert.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>

using scl::file;
using scl::directory;
using scl::file_reader;
using scl::string16;
using scl::string32;
using scl::string128;

 void testFile1()
{
	file f;
	assert(f.open("ansi.txt", "rb"));
	byte buffer[32] = { 0 };
	f.read(buffer, sizeof(buffer) - 1);
	string32 s = reinterpret_cast<char*>(buffer);
	assert(s == "abcdefg");
	f.close();

	//f.open("utf16.txt", "rt");
	//wstring64 utf16;
	//f.readString(utf16.c_str(), utf16.max_size());
	//assert(utf16 == L"abc我是def六g");
	//f.close();
}

 void testFile2()
{
	//file f;
	//f.open(L"testSize.txt", L"w+");
	//wstring64 testText = L"123456";
	//f.writeString(testText.c_str(), testText.length());
	//assertf(f.size() == 14, "f.size() = %d", f.size());
	//f.writeString(testText.c_str(), testText.length());
	//assertf(f.size() == 26,  "f.size() = %d", f.size());
}

void testFile3()
{
	file f;
	f.open(_ANSI("测试.txt"), "rb");
	char s[16] = { 0 };
	f.read(s, 16);
	assert(::strcmp(s, "abc") == 0);
}


 void testFile()
{
	testFile1();
	testFile2();
	testFile3();
	testFileReader();

	printf("test file \t\tOK!\n");
}

 void testDirectory()
{
	directory dir;
	dir.open("./testDirectory");
	dir.next();
	assert(dir.current().name() == ".");
	dir.next();
	assert(dir.current().name() == "..");
	//dir.next();
	//assert(dir.current().name() == ".svn");
	dir.next();
	assert(dir.current().name() == "a.txt");
	dir.next();
	assert(dir.current().name() == "dir");
	dir.next();
	assert(dir.current().name() == _ANSI("文件夹"));
	dir.next();
	assert(dir.current().name() == _ANSI("测试文件.txt"));
	printf("test directory \t\tOK!\n");
}

void testFileReader()
{
	file_reader fr1(10, 10);
	fr1.open("file_reader_test1.txt");

	fr1.nextline();
	assert(::strcmp(fr1.currentline(), "123456789") == 0);

	fr1.nextline();
	assert(::strcmp(fr1.currentline(), "345") == 0);

	fr1.nextline();
	assert(::strcmp(fr1.currentline(), "6789") == 0);

	fr1.clear();
	fr1.open("file_reader_test1.txt");

	fr1.nextline();
	assert(::strcmp(fr1.currentline(), "123456789") == 0);

	fr1.nextline();
	assert(::strcmp(fr1.currentline(), "345") == 0);

	fr1.nextline();
	assert(::strcmp(fr1.currentline(), "6789") == 0);


	file_reader fr2(9, 10);
	fr2.open("file_reader_test1.txt");

	fr2.nextline();
	assert(::strcmp(fr2.currentline(), "123456789") == 0);

	fr2.nextline();
	assert(::strcmp(fr2.currentline(), "345") == 0);

	fr2.nextline();
	assert(::strcmp(fr2.currentline(), "6789") == 0);

	file_reader fr3;
	fr3.open("file_reader_test1.txt");
	fr3.nextline();
	assert(::strcmp(fr3.currentline(), "123456789") == 0);

	fr3.nextline();
	assert(::strcmp(fr3.currentline(), "345") == 0);

	fr3.nextline();
	assert(::strcmp(fr3.currentline(), "6789") == 0);


}

 void testExtractFileName()
 {
	 string128 s = "c:\\dir1\\dir2\\file";
	 normalize_path(s.pstring());
	 assert(s == "c:/dir1/dir2/file/");

	 s = "c:\\dir1\\dir2\\file";
	 extract_path(s.pstring());
	 assert(s == "c:\\dir1\\dir2\\");

	 s = "a.txt";
	 extract_path(s.pstring());
	 assert(s == "./");

	 s = "c:\\dir1\\dir2\\file.txt";
	 extract_filename(s.pstring());
	 assert(s == "file.txt");

	 s = "c:\\dir1\\dir2\\file.txt";
	 extract_filename(s.pstring(), false);
	 assert(s == "file");

	 s = "c:\\dir1\\dir2\\file.txt";
	 extract_fileext(s.pstring());
	 assert(s == "txt");

	 string16 ext;
	 s = "c:\\dir1\\dir2\\file.txt";
	 extract_fileext_to(s.pstring(), ext.pstring());
	 assert(s == "c:\\dir1\\dir2\\file.txt");
	 assert(ext == "txt");

	 s = "ui/loading3";
	 extract_filename(s.pstring(), false);
	 assert(s == "loading3");
 }


