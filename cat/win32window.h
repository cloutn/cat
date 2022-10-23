////////////////////////////////////////////////////////////
//	win32Window
//	
//	2010.11.20 caolei
////////////////////////////////////////////////////////////
#pragma once

//#include "scl/type.h"
#include "scl/function.h"

#include <stdint.h>

//#ifdef _WIN64
//typedef __int64 ptr_int; 
//#else
//typedef __int32 ptr_int;
//#endif

namespace cat {

class Win32Window
{
public:
	//typedef bool (*EventHandlerFuncT)(void* caller, void* hWnd, uint32 message, uint32 wParam, uint32 lParam);

	//class EventHandler
	//{
	//public:
	//	EventHandler		() : caller(NULL), func(NULL) {}
	//	EventHandler		(void* _caller, EventHandlerFuncT _func) : caller(_caller), func(_func) {}
	//	bool operator==		(const EventHandler& other) const { return caller == other.caller && func == other.func; }

	//	void*				caller;
	//	EventHandlerFuncT	func;
	//};

public:
	Win32Window();
	~Win32Window();

	bool	init					(const int width, const int height, const wchar_t* const titleName = L"main", const wchar_t* const szIconName = L"", bool enableDpiAwareness = false);
	bool	hasInit					() { return m_windowHandle != NULL; }
	
	bool	run						();

	bool	registerEventHandler	(void* caller, scl::class_function_ptr func);
	bool	unregisterEventHandler	(void* caller, scl::class_function_ptr func);
	bool	postEvent				(void* hWnd, uint32_t message, intptr_t wParam, intptr_t lParam);
	bool	IsForegroundWindow		();

	int		getWidth				() { return m_width; };
	int		getHeight				() { return m_height; };
	void*	getHandle				() { return m_windowHandle; };
	void*	getInstance				() { return m_hInstance; }


private:
	static intptr_t __stdcall WndProc(void* hWnd, uint32_t message, intptr_t wParam, intptr_t lParam);

private:
	void*				m_windowHandle;
	void*				m_hInstance;

	int					m_width;
	int					m_height;
	
	//事件处理器列表
	static const int	MAX_EVENT_HANDLER = 256;
	typedef				scl::any_class_function<bool, void*, uint32_t, intptr_t, intptr_t> EventHandlerFuncT;

	EventHandlerFuncT	m_eventHandlers[MAX_EVENT_HANDLER];
	int					m_eventHandlerCount;
};

}	//namespace cat


