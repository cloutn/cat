////////////////////////////////////////////////////////////
//	 win32下窗口类
//	
//	2010.11.20 caolei
////////////////////////////////////////////////////////////

#include "cat/win32Window.h"

#include <windows.h>
#include <tchar.h>
#include <assert.h>

namespace cat {

//typedef DPI_AWARENESS_CONTEXT(WINAPI* PFN_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT); // User32.lib + dll, Windows 10 v1607+ (Creators Update)
void EnableDpiAwareness()
{
	::SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	//static HINSTANCE user32_dll = ::LoadLibraryA("user32.dll"); // Reference counted per-process
	//if (PFN_SetThreadDpiAwarenessContext SetThreadDpiAwarenessContextFn = (PFN_SetThreadDpiAwarenessContext)::GetProcAddress(user32_dll, "SetThreadDpiAwarenessContext"))
	//{
	//	SetThreadDpiAwarenessContextFn();
	//	return;
	//}

#if _WIN32_WINNT >= 0x0600
	::SetProcessDPIAware();
#endif
}

Win32Window::Win32Window() :
	m_windowHandle		(NULL),
	m_hInstance			(NULL),
	//m_width				(0),
	//m_height			(0),
	m_eventHandlerCount	(0)
{
	//for (int i = 0; i < MAX_EVENT_HANDLER; ++i)
	//{
	//	m_eventHandlers[i] = NULL;
	//}
}

bool Win32Window::init(const int posx, const int posy, const int width, const int height, const wchar_t* const titleName, const wchar_t* const szIconName, bool enableDpiAwareness) //  titleName = "main"
{
	//不允许反复初始化
	if (hasInit())
	{
		assert(false);
		return false;
	}

	if (enableDpiAwareness)
		EnableDpiAwareness();

	HINSTANCE hInstance = ::GetModuleHandle(0);
	m_hInstance = hInstance;

	// 计算获得最终的窗口尺寸
	::RECT rect;
	::SetRect(
		&rect, 
		posx,						//  int x,
		posy,						//  int y,
		posx + width,				//  int x + width,
		posy + height);				//  int y + height,

	::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	//注册窗口类
	const TCHAR szWindowClass[] = _T("MainWindowClass");			// 主窗口类名
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= reinterpret_cast<WNDPROC>(&Win32Window::WndProc);
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= ::LoadIcon(hInstance, szIconName);
	wcex.hCursor		= ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= 0;
	::RegisterClassEx(&wcex);

	//标题栏文本
	//const TCHAR szTitle[]		= _T("main");
	//创建窗口
	//m_windowHandle = ::CreateWindow(		
	//	szWindowClass,			//  LPCTSTR lpClassName,
	//	titleName,				//  LPCTSTR lpWindowName,
	//	WS_OVERLAPPEDWINDOW,	//  DWORD dwStyle,
	//	rect.left,				//  int x, or CW_USEDEFAULT for auto position
	//	rect.top,				//  int y,
	//	rect.right - rect.left,	//  int nWidth,
	//	rect.bottom - rect.top,	//  int nHeight,
	//	NULL,					//  HWND hWndParent,
	//	NULL,					//  HMENU hMenu,
	//	hInstance,				//  HINSTANCE hInstance,
	//	this);					//  LPVOID lpParam

	//创建窗口
	m_windowHandle = CreateWindowEx(
		0,
		szWindowClass,
		titleName,
		WS_OVERLAPPEDWINDOW,	//	(WS_POPUPWINDOW | WS_THICKFRAME | WS_VISIBLE | WS_CLIPCHILDREN),
		rect.left,				//  int x, or CW_USEDEFAULT for auto position
		rect.top,				//  int y,
		rect.right - rect.left,	//  int nWidth,
		rect.bottom - rect.top,	//  int nHeight,
		NULL,
		NULL,
		hInstance,				//  HINSTANCE hInstance,
		this);

	if (!m_windowHandle)
		return false;

	::ShowWindow(static_cast<HWND>(m_windowHandle), SW_SHOW);
	::UpdateWindow(static_cast<HWND>(m_windowHandle));

	return true;
}
	
bool Win32Window::run() 
{
	::MSG msg;
	while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			return false;
	}
	return true;
}

intptr_t __stdcall Win32Window::WndProc(void* hWnd, uint32_t message, intptr_t wParam, intptr_t lParam)
{
	//首先处理窗口的WM_DESTROY消息
	if (message == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	//TODO 第一条 WM_GETMINMAXINFO 消息无法发送给 Win32Window，
	//因为在CreateWindow函数中，WM_GETMINMAXINFO消息先于WM_NCCREATE到达
	if (message == WM_NCCREATE)
	{
		//将窗口的this指针绑定到HWND的user data上
		::SetWindowLongPtr(
			static_cast<HWND>(hWnd), 
			GWLP_USERDATA, 
			reinterpret_cast<LONG_PTR>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams));
	}
	Win32Window* pWindow = reinterpret_cast<Win32Window*>(::GetWindowLongPtr(static_cast<HWND>(hWnd), GWLP_USERDATA));
	if (NULL == pWindow)
	{
		if (message != WM_GETMINMAXINFO)
			assert(false);
		return ::DefWindowProc(static_cast<HWND>(hWnd), message, wParam, lParam);
	}
	//遍历所有handler，尝试处理消息
	if (pWindow->postEvent(hWnd, message, wParam, lParam)) 
		return 0;

	return ::DefWindowProc(static_cast<HWND>(hWnd), message, wParam, lParam);
}

bool Win32Window::registerEventHandler(EventHandlerFuncT eventHandler)
{
	for (int i = 0; i < m_eventHandlerCount; ++i)
	{
		if (m_eventHandlers[i] == eventHandler)
		{
			assert(0);
			return false;
		}
	}
	m_eventHandlers[m_eventHandlerCount] = eventHandler;
	m_eventHandlerCount++;
	return true;
}

bool Win32Window::unregisterEventHandler(EventHandlerFuncT eventHandler)
{
	for (int i = 0; i < m_eventHandlerCount; ++i)
	{
		if (m_eventHandlers[i] == eventHandler)
		{
			if (i != m_eventHandlerCount - 1)
			{
				m_eventHandlers[i] = m_eventHandlers[m_eventHandlerCount - 1];
			}
			m_eventHandlerCount--;
			break;
		}
	}
	return true;
}

bool Win32Window::postEvent(void* hWnd, uint32_t message, intptr_t wParam, intptr_t lParam)
{
	for (int i = 0; i < m_eventHandlerCount; ++i)
	{
		EventHandlerFuncT& eventHandler = m_eventHandlers[i];
		int handled = eventHandler(hWnd, message, wParam, lParam);
		if (handled)
		{
			//事件已被处理
			return true;
		}
	}
	return false;
}

bool Win32Window::IsForegroundWindow()
{
	return ::GetForegroundWindow() == m_windowHandle;
}

int Win32Window::getWidth() const
{
	return getRect().width();
}

int Win32Window::getHeight() const
{
	return getRect().height();
}

int Win32Window::getPositionX() const
{
	return getRect().left;
}

int Win32Window::getPositionY() const
{
	return getRect().top;
}

scl::rect Win32Window::getRect() const
{
	//RECT windowRect;
	//::GetWindowRect((HWND)m_windowHandle, &windowRect);

	BOOL result = 0;

	POINT pos;
	memset(&pos, 0, sizeof(pos));
	result = ::ClientToScreen((HWND)m_windowHandle, &pos);
	assert(result);

	RECT rect;
	memset(&rect, 0, sizeof(rect));
	result = ::GetClientRect((HWND)m_windowHandle, &rect);
	assert(result);

	scl::rect rr(pos.x, pos.y, pos.x + rect.right, pos.y + rect.bottom);
	return rr;
}

Win32Window::~Win32Window()
{
	::DestroyWindow(static_cast<HWND>(m_windowHandle));
}

}	//namespace cat


