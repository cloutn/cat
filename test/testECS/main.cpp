
#include "scl/type.h"

#ifdef _WIN32
#include <crtdbg.h>
#include <Windows.h> //#include "vld_runtime/include/vld.h" //#include <C:\\Program Files (x86)\\Visual Leak Detector\\include\\vld.h>
#endif


class World
{

};

typedef uint32 Entity;

class Component
{
public:
};


int main()
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//如果发生泄漏，请将泄漏的内存序号填写在下面
	//_CrtSetBreakAlloc(1535);
#endif
	printf("\n\n****************** test raw ****************** \n\n");

	test_write_raw();

	test_read_raw();

	printf("\n\n****************** test cat::yaml ******************\n\n");

	test_write();

	test_read();

	getchar();

	return 0;
}


