
#include "testLog.h"

#include <stdio.h>

#include "scl/log.h"
#include "scl/log_file.h"

using scl::log;
using scl::log_file;

void testLog()
{
	log_debug << "this is a debug log test!" << scl::endl;
	log_info << "this is a info log test!" << scl::endl;
	log_warn << "this is a warn log test!" << scl::endl;
	log_error << "this is a error log test!" << scl::endl;

	//测试static的write方法
	//log::write("testLog.txt", "lalala");

	//测试memory log
	log_file lf(1983);
	lf.open("memoryLog.txt");
	lf.write("my year is %d", 1983);
}
