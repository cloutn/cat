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

	void	OnGUI				();
	void	OnEvent				(void* hWnd, uint32 message, uint32 wParam, uint32 lParam);
	bool	WantCaptureKeyboard	();
	bool	WantCaptureMouse	();

private:
	void	_onGUIScene			(const int sceneIndex, bool& isContextMenuOpen);
	void	_onGUIObject		(Object* const object, bool& isContextMenuOpen);
	void	_onGUIProperty		(Object* const object);

private:
	Client*		m_client;	
	Object*		m_selectObject;

}; // class MainGUI


} // namespace cat


