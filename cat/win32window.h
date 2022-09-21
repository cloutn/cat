////////////////////////////////////////////////////////////
//	win32Window
//	
//	2010.11.20 caolei
////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"

#ifdef _WIN64
typedef __int64 ptr_int; 
#else
typedef __int32 ptr_int;
#endif

namespace cat {

class Win32Window
{
public:
	typedef bool (*EventHandlerFuncT)(void* caller, void* hWnd, uint32 message, uint32 wParam, uint32 lParam);

	class EventHandler
	{
	public:
		EventHandler		() : caller(NULL), func(NULL) {}
		EventHandler		(void* _caller, EventHandlerFuncT _func) : caller(_caller), func(_func) {}
		bool operator==		(const EventHandler& other) const { return caller == other.caller && func == other.func; }

		void*				caller;
		EventHandlerFuncT	func;
	};

public:
	Win32Window();
	~Win32Window();

	bool	init					(const int width, const int height, const wchar* const titleName = L"main", const wchar* const szIconName = L"", bool enableDpiAwareness = false);
	bool	hasInit					() { return m_windowHandle != NULL; }
	
	bool	run						();

	bool	registerEventHandler	(void* caller, EventHandlerFuncT func);
	bool	unregisterEventHandler	(void* caller, EventHandlerFuncT func);
	bool	postEvent				(void* hWnd, uint32 message, uint32 wParam, uint32 lParam);

	int		getWidth				() { return m_width; };
	int		getHeight				() { return m_height; };
	void*	getHandle				() { return m_windowHandle; };
	void*	getInstance				() { return m_hInstance; }


private:
	static ptr_int __stdcall WndProc(void* hWnd, uint32 message, ptr_int wParam, ptr_int lParam);

private:
	void*				m_windowHandle;
	void*				m_hInstance;

	int					m_width;
	int					m_height;
	
	//事件处理器列表
	static const int	MAX_EVENT_HANDLER = 256;
	EventHandler		m_eventHandlers[MAX_EVENT_HANDLER];
	int					m_eventHandlerCount;
};

}	//namespace cat


