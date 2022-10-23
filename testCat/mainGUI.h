
#pragma once

#include "scl/type.h"
#include "scl/function.h"

namespace cat {

class Client;
class Object;

class MainGUI
{
public:
	MainGUI();

	void	init				(Client* client);
	void	release				();

	void	onGUI				();
	void	Render				();
	void	onEvent				(void* hWnd, uint32 message, uint32 wParam, uint32 lParam);
	bool	wantCaptureKeyboard	();
	bool	wantCaptureMouse	();

	void	setDebugButton1ClickEvent(void* caller, scl::class_function_ptr func);

private:
	void	_windowScene		();
	void	_windowProperty		(Object* const object);
	void	_windowDebug		();
	void	_windowDeviceInfo	();

	void	_onGUIScene			(const int sceneIndex, bool& isContextMenuOpen);
	void	_onGUIObject		(Object* const object, bool& isContextMenuOpen);
	void	_beginFrame			();
	void	_endFrame			();

private:
	Client*						m_client;	
	Object*						m_selectObject;

	scl::any_class_function<void>	m_debugButton1ClickFunc;

}; // class MainGUI


} // namespace cat


