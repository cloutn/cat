#pragma once

#include "scl/type.h"

namespace cat {

class Client;
class Object;

typedef void (*ButtonClickFunc)(void* caller);

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

	void	setDebugButton1ClickEvent	(void* caller, ButtonClickFunc func);

private:
	void	_windowScene		();
	void	_windowProperty		(Object* const object);
	void	_windowDebug		();

	void	_onGUIScene			(const int sceneIndex, bool& isContextMenuOpen);
	void	_onGUIObject		(Object* const object, bool& isContextMenuOpen);
	void	_beginFrame			();
	void	_endFrame			();

private:
	Client*						m_client;	
	Object*						m_selectObject;

	void*						m_debugButton1ClickCaller;
	ButtonClickFunc				m_debugButton1ClickFunc;

}; // class MainGUI


} // namespace cat


