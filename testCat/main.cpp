

#include "./client.h"

#include "scl/log.h"

#ifdef _WIN32
#include <crtdbg.h>
#include <Windows.h>
//#include "vld_runtime/include/vld.h"
//#include <C:\\Program Files (x86)\\Visual Leak Detector\\include\\vld.h>
#endif

//uint8 PartialHidden = 0;
//
//void set_hide(const bool bInHidden, const int PartialIndex)
//{
//	int shift = PartialIndex;
//	shift = scl::clamp(shift, 0, 7);
//	if (bInHidden)
//	{
//		PartialHidden |= 0x01 << shift;		
//	}
//	else
//	{
//		PartialHidden &= ~(0x01 << shift);
//	}
//
//	bool isHidden = ((PartialHidden & 0x03) == 0x03);
//	printf("isHidden = %d ,partial hidden = %d\n", isHidden, PartialHidden);
//}


void start_client()
{
	//set_hide(true, 0);
	//set_hide(true, 1);
	//set_hide(false, 0);
	//set_hide(true, 0);
	//set_hide(false, 1);
	//set_hide(false, 0);

	//int a = 0;

	//a |= (0x01 << 3);
	//a |= (0x01 << 2);
	//a &= ~(0x01 << 3);
	//a |= (0x01 << 1);
	//int b = (a & 0x03);
	//printf("a = %d, b = %d\n", a, b);

	cat::Client* c = new cat::Client();
	c->init();
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

	start_client();

	scl::log::release();

	return 0;
}

