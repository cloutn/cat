
#include "./testBox.h"

#include <stdio.h>

#ifdef _WIN32
#include <crtdbg.h>
#include <Windows.h>
#endif

int main()
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//如果发生泄漏，请将泄漏的内存序号填写在下面
	//_CrtSetBreakAlloc(1535);
#endif

	testBox();

	getchar();

	return 0;
}


