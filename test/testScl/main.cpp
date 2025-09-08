
#include "testArray.h"
#include "testList.h"
#include "testMemory.h"
#include "testString.h"
#include "testPString.h"
#include "testWString.h"
#include "testPWString.h"
#include "testFile.h"
#include "testStack.h"
#include "testHashTable.h"
#include "testTree.h"
#include "testTime.h"
#include "testLog.h"
#include "testRingBuffer.h"
#include "testRingQueue.h"
#include "testThread.h"
#include "testIniFile.h"
#include "testBitset.h"
#include "testStringFunction.h"
#include "testBigInt.h"
#include "testHeap.h"
#include "testPath.h"
#include "testTypeTraits.h"
#include "testString2025.h"

#include "scl/allocator.h"
#include "scl/list.h"
#include "scl/tree.h"
#include "scl/log.h"

#include <stdio.h>
#include <locale.h>

#ifdef _WIN32
#include "crtdbg.h"
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//如果发生泄漏，请将泄漏的内存序号填写在下面
	//_CrtSetBreakAlloc(366);
#endif

	typedef scl::single_allocator<scl::list_node<int> > IntNodeAlloc;
	IntNodeAlloc::init(100);

	scl::single_allocator<scl::tree_node<int, int> >::init(128);

	//显示wchar
	setlocale(LC_ALL, "");

	testArray();
	testList();
	testMemory();
	testPool();
	testStack();
	testHeap();
	testTree();
	testHashTable();
	testRingBuffer();
	testRingQueue();
	testVRingQueue();
	testBitset();

	// string
	old_testString();
	testString();
	testPString();
	testWString();
	testPWString();
	testStringFunction();
	testString2025();

	testFile();
	testDirectory();
	testExtractFileName();
	testBigInt();
	testIniFile();
	testIniWriter();
	testPath();
	testTypeTraits();
	//

	////以下为非常规测试
	testLog();
	testThread();
	testTime();
	

	scl::single_allocator<scl::tree_node<int, int> >::release();
	IntNodeAlloc::release();
	scl::log::release();

	printf("finished!\n");

	getchar();

	return 0;
}

