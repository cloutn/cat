
#include "./client.h"

//#include "libimg/image.h"

#include "scl/log.h"

// ####################################
#ifdef _WIN32
#include <crtdbg.h>
#include <Windows.h>
//#include "vld_runtime/include/vld.h"
//#include <C:\\Program Files (x86)\\Visual Leak Detector\\include\\vld.h>
#endif
// ####################################

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
	//如果发生泄漏，请将泄漏的内存序号填写在下面
	//_CrtSetBreakAlloc(1535);
#endif

	//FILE* f = fopen("e:/1.bmp", "wb");
	//uint* data = new uint[100 * 100];
	//for (int i = 0; i < 100; ++i)
	//{
	//	for (int j = 0; j < 100; ++j)
	//	{
	//		data[i * 100 + j] = i * i * j * j;
	//	}
	//}
	//img::save_bmp(f, 100, 100, 0, (uint8*)data);
	//fclose(f);
	//delete[] data;

	test();

	scl::log::release();

	return 0;
}

