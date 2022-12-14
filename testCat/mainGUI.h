
#pragma once

#include "scl/array.h"
#include "scl/function.h"
#include "scl/vector.h"
#include "scl/type.h"

namespace cat {

class Client;
class Object;

class GUIEvent
{
public:

};

enum GUI_EVENT
{
	GUI_EVENT_DEBUG_BUTTON_CLICK,

	GUI_EVENT_COUNT,
};

typedef scl::class_function<bool (GUIEvent&)> GUIEventFuncT;

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
	void	registerEvent		(GUI_EVENT event, GUIEventFuncT pfunc);

private:
	void	_windowScene		();
	void	_windowProperty		(Object* const object);
	void	_windowDebug		();
	void	_windowDeviceInfo	();

	void	_onGUIScene			(const int sceneIndex, bool& isContextMenuOpen);
	void	_onGUIObject		(Object* const object, bool& isContextMenuOpen);
	void	_beginFrame			();
	void	_endFrame			();

	void	_fireEvent			(GUI_EVENT event, GUIEvent& eventArg);


private:
	Client*						m_client;	
	Object*						m_selectObject;


	scl::array<GUIEventFuncT, GUI_EVENT_COUNT> m_events;

}; // class MainGUI


} // namespace cat


