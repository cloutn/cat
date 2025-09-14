////////////////////////////////////////////////////////////
//	win32Window
//	
//	2010.11.20 caolei
////////////////////////////////////////////////////////////
#pragma once

#include "scl/function.h"
#include "scl/rect.h"

#include <stdint.h>

namespace cat {

class Win32Window
{
public:
	Win32Window();
	~Win32Window();

	typedef scl::class_function<bool (void*, uint32_t, intptr_t, intptr_t)> EventHandlerFuncT;

	bool		init					(const int posx, const int posy, const int width, const int height, const wchar_t* const titleName = L"main", const wchar_t* const szIconName = L"", bool enableDpiAwareness = false);
	bool		hasInit					() { return m_windowHandle != NULL; }
	bool		run						();
	bool		registerEventHandler	(EventHandlerFuncT func);
	bool		unregisterEventHandler	(EventHandlerFuncT func);
	bool		postEvent				(void* hWnd, uint32_t message, intptr_t wParam, intptr_t lParam);
	bool		IsForegroundWindow		();
	int			getWidth				() const;
	int			getHeight				() const;
	int			getPositionX			() const;
	int			getPositionY			() const;
	scl::rect	getRect					() const;
	void*		getHandle				() { return m_windowHandle; };
	void*		getInstance				() { return m_hInstance; }

private:
	static intptr_t __stdcall WndProc(void* hWnd, uint32_t message, intptr_t wParam, intptr_t lParam);

private:
	void*				m_windowHandle;
	void*				m_hInstance;

	//int					m_width;
	//int					m_height;
	
	//事件处理器列表
	static const int	MAX_EVENT_HANDLER = 128;

	EventHandlerFuncT	m_eventHandlers[MAX_EVENT_HANDLER];
	int					m_eventHandlerCount;
};

}	//namespace cat


