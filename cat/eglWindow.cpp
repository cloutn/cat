
#include "cat/eglWindow.h"


#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdio.h> // for print_info

#ifdef _WIN32
#include <Windows.h>
#endif

namespace cat {

bool eglWindow_create(void* hwnd, EGLDisplay& display, EGLSurface& surface, EGLContext& context)
{
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (display == EGL_NO_DISPLAY) 
		return false;

	EGLBoolean r = EGL_FALSE;
	r = eglInitialize(display, NULL, NULL);
	if (!r)
		return false;

	r = eglBindAPI(EGL_OPENGL_ES_API);
	if (!r)
		return false;

	//choose config
	int attr[] = 
	{
		EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES3_BIT_KHR,
		EGL_RED_SIZE,			8,
		EGL_GREEN_SIZE,			8,
		EGL_BLUE_SIZE,			8,
		EGL_ALPHA_SIZE,			8,
		EGL_DEPTH_SIZE,			24,
		EGL_NONE,
	};
	EGLint		config_count	= 0;
	EGLConfig	config			= NULL;
	r = eglChooseConfig(display, attr, &config, 1, &config_count);
	if (!r || config_count == 0)
		return false;

	// surface
	surface = eglCreateWindowSurface(display, config, static_cast<HWND>(hwnd), NULL);
	if (surface == EGL_NO_SURFACE) 
		return false;

	// context
	EGLint contextAttributes[]	= { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
	context		= eglCreateContext(display, config, NULL, contextAttributes);
	if (context == EGL_NO_CONTEXT) 
		return false;
	r = eglMakeCurrent(display, surface, surface, context);
	if (!r)
		return false;

	return true;
}

void eglWindow_swapBuffer(EGLDisplay& display, EGLSurface& surface)
{
	eglSwapBuffers(display, surface);
}

bool EGLWindow::create(void* hwnd)
{
	bool r = eglWindow_create(hwnd, m_display, m_surface, m_context);
	eglQuerySurface(m_display, m_surface, EGL_HEIGHT,	&m_height);
	eglQuerySurface(m_display, m_surface, EGL_WIDTH,	&m_width);
	return r;
}

void EGLWindow::swap()
{
	return eglWindow_swapBuffer(m_display, m_surface);
}

void EGLWindow::print_info() const
{
	printf("EGL Version\t= %s\n"	, eglQueryString(m_display, EGL_VERSION));
	printf("EGL Vendor\t= %s\n"		, eglQueryString(m_display, EGL_VENDOR));
	//cout << "EGL Client APIs : \n" << eglQueryString(eglDisplay, EGL_CLIENT_APIS) << "\n";
	//cout << "EGL Extensions : \n" << eglQueryString(eglDisplay, EGL_EXTENSIONS) << "\n"
}

void EGLWindow::update_size()
{
	eglQuerySurface(m_display, m_surface, EGL_HEIGHT, &m_height);
	eglQuerySurface(m_display, m_surface, EGL_WIDTH,	&m_width);
}

void EGLWindow::update_size(const int width, const int height)
{
	m_width = width;
	m_height = height;
}

EGLWindow::~EGLWindow()
{
}

void EGLWindow::release()
{
	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	if (m_context != EGL_NO_CONTEXT)
		eglDestroyContext(m_display, m_context);
	if (m_surface != EGL_NO_SURFACE)
		eglDestroySurface(m_display, m_surface);
	eglTerminate(m_display);
	eglReleaseThread();
}

} // namespace cat


