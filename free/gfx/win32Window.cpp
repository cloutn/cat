////////////////////////////////////////////////////////////
//	 win32�´�����
//	
//	2010.11.20 caolei
////////////////////////////////////////////////////////////

#include "gfx/win32Window.h"

//#include "scl/type.h"
//#include "scl/log.h"

#include <windows.h>
#include <tchar.h>
#include <assert.h>

namespace gfx {

//typedef DPI_AWARENESS_CONTEXT(WINAPI* PFN_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT); // User32.lib + dll, Windows 10 v1607+ (Creators Update)
void EnableDpiAwareness()
{
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
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
	m_width				(0),
	m_height			(0),
	m_eventHandlerCount	(0)
{
	//for (int i = 0; i < MAX_EVENT_HANDLER; ++i)
	//{
	//	m_eventHandlers[i] = NULL;
	//}
}

bool Win32Window::init(const int width, const int height, const wchar* const titleName, const wchar* const szIconName, bool enableDpiAwareness) //  titleName = "main"
{
	//����������ʼ��
	if (hasInit())
	{
		assert(false);
		return false;
	}

	if (enableDpiAwareness)
		EnableDpiAwareness();

	HINSTANCE hInstance = GetModuleHandle(0);
	m_hInstance = hInstance;

	//��ʼ����Ļ�ߴ���ر���
	m_width		= width;
	m_height	= height;

	// ���������յĴ��ڳߴ�
	::RECT window_rect;
	::SetRect(&window_rect, 
		0,						//  int x,
		0,						//  int y,
		m_width,				//  int x + nWidth,
		m_height);				//  int y + nHeight,
	::AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);

	//ע�ᴰ����
	const TCHAR szWindowClass[] = _T("MainWindowClass");			// ����������
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= reinterpret_cast<WNDPROC>(&Win32Window::WndProc);
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, szIconName);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= 0;
	RegisterClassEx(&wcex);

	//�������ı�
	//const TCHAR szTitle[]		= _T("main");
	//��������
	m_windowHandle = ::CreateWindow(		
		szWindowClass,			//  LPCTSTR lpClassName,
		titleName,				//  LPCTSTR lpWindowName,
		WS_OVERLAPPEDWINDOW,	//  DWORD dwStyle,
		CW_USEDEFAULT,			//  int x,
		window_rect.top,		//  int y,
		window_rect.right - window_rect.left,	//  int nWidth,
		window_rect.bottom - window_rect.top,	//  int nHeight,
		NULL,					//  HWND hWndParent,
		NULL,					//  HMENU hMenu,
		hInstance,				//  HINSTANCE hInstance,
		this);					//  LPVOID lpParam

	if (!m_windowHandle)
		return false;

	ShowWindow(static_cast<HWND>(m_windowHandle), SW_SHOW);
	UpdateWindow(static_cast<HWND>(m_windowHandle));

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

ptr_int __stdcall Win32Window::WndProc(void* hWnd, uint32 message, ptr_int wParam, ptr_int lParam)
{
	//���ȴ����ڵ�WM_DESTROY��Ϣ
	if (message == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	//TODO ��һ��WM_GETMINMAXINFO��Ϣ�޷����͸�Win32Window��
	//��Ϊ��CreateWindow�����У�WM_GETMINMAXINFO��Ϣ����WM_NCCREATE����
	if (message == WM_NCCREATE)
	{
		//�����ڵ�thisָ��󶨵�HWND��user data��
		::SetWindowLongPtr(
			static_cast<HWND>(hWnd), 
			GWLP_USERDATA, 
			reinterpret_cast<LONG_PTR>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams));
	}
	//��HWND�л�ȡWin32Window��ָ��
	Win32Window* pWindow = reinterpret_cast<Win32Window*>(
		GetWindowLongPtr(static_cast<HWND>(hWnd), GWLP_USERDATA));
	if (NULL == pWindow)
	{
		if (message != WM_GETMINMAXINFO)
			assert(false);
		return DefWindowProc(static_cast<HWND>(hWnd), message, wParam, lParam);
	}
	//��������handler�����Դ�����Ϣ
	if (pWindow->postEvent(hWnd, message, wParam, lParam)) 
		return 0;

	return DefWindowProc(static_cast<HWND>(hWnd), message, wParam, lParam);
}

bool Win32Window::registerEventHandler(void* caller, EventHandlerFuncT func)
{
	EventHandler eventHandler(caller, func);
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

bool Win32Window::unregisterEventHandler(void* caller, EventHandlerFuncT func)
{
	EventHandler eventHandler(caller, func);

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

bool Win32Window::postEvent(void* hWnd, uint32 message, uint32 wParam, uint32 lParam)
{
	for (int i = 0; i < m_eventHandlerCount; ++i)
	{
		EventHandler& eventHandler = m_eventHandlers[i];
		if (eventHandler.func(eventHandler.caller, hWnd, message, wParam, lParam))
		{
			//�¼��ѱ�����
			return true;
		}
	}
	return false;
}

Win32Window::~Win32Window()
{
	DestroyWindow(static_cast<HWND>(m_windowHandle));
}



}	//namespace gfx 


