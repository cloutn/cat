#pragma once

#include "scl/type.h"

namespace cat {

class Client;
class Object;

class MainGUI
{
public:
	MainGUI();

	void	init				(Client* client);
	void	_initIMGUI			();
	void	release				();

	void	onGUI				();
	void	onEvent				(void* hWnd, uint32 message, uint32 wParam, uint32 lParam);
	bool	wantCaptureKeyboard	();
	bool	wantCaptureMouse	();

private:
	void	_windowScene		();
	void	_windowProperty		(Object* const object);
	void	_windowDebug		();

	void	_onGUIScene			(const int sceneIndex, bool& isContextMenuOpen);
	void	_onGUIObject		(Object* const object, bool& isContextMenuOpen);
	void	_beginFrame			();
	void	_endFrame			();

private:
	Client*		m_client;	
	Object*		m_selectObject;

}; // class MainGUI


} // namespace cat


