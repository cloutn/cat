
#include "./client.h"

#include "scl/log.h"
#ifdef _WIN32
#include <crtdbg.h>
#include <Windows.h>
//#include "vld_runtime/include/vld.h"
//#include <C:\\Program Files (x86)\\Visual Leak Detector\\include\\vld.h>
#endif

void test()
{
	cat::Client* c = new cat::Client();
	c->init(1280, 768);
	c->run();
	delete c;
}


int main()
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//�������й©���뽫й©���ڴ������д������
	//_CrtSetBreakAlloc(1535);
#endif

	test();

	scl::log::release();

	return 0;
}

