#pragma once


typedef void *EGLContext;
typedef void *EGLDisplay;
typedef void *EGLSurface;

#define EGL_NO_CONTEXT			((EGLContext)0)
#define EGL_NO_DISPLAY			((EGLDisplay)0)
#define EGL_NO_SURFACE			((EGLSurface)0)

namespace cat {

bool eglWindow_create		(void* hwnd, EGLDisplay& display, EGLSurface& surface, EGLContext& context);
void eglWindow_swapBuffer	(EGLDisplay& display, EGLSurface& surface);

class EGLWindow
{
public:
	EGLWindow() : m_display(EGL_NO_DISPLAY), m_surface(EGL_NO_SURFACE), m_context(EGL_NO_CONTEXT), m_height(0), m_width(0) {}
	~EGLWindow();

	bool	create		(void* hwnd);
	void	swap		();
	void	update_size	();
	void	update_size	(const int width, const int height);
	int		height		() const { return m_height; }
	int		width		() const { return m_width; }
	void	print_info	() const;
	bool	is_init	() const { return m_display != EGL_NO_DISPLAY; }
	void	release();

private:
	EGLDisplay	m_display;
	EGLSurface	m_surface;
	EGLContext	m_context;
	int			m_height;
	int			m_width;
};

} // namespace cat

