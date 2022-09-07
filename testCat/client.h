
#pragma once

#include "scl/type.h"
#include "scl/vector.h"

#include "cat/uiRenderOpenGL.h"
#include "catVulkanRender/vulkanRender.h"

#include "cat/def.h"

#include "./config.h"
#include "./mainGUI.h"

#ifdef SCL_WIN
#include "gfx/win32window.h"
#endif

struct cgltf_data;

namespace cat {

class Object;
class Camera;
class Material;
class Primitive;
class Env;
class Scene;
class Animation;

class Client
{
public:
	Client();
	virtual ~Client();

	static Client&			inst				();
	void					init				(const int width, const int height);

	void					loadGltf			(const char* const filename);
	void					updateAnimation		(const double diff);
	Object*					findObject			(const char* const objectName);
	Object*					getObject			(int sceneIndex, int index);

	Config&					config				() { return m_config; }
	scl::varray<Scene*>&	scenes				() { return m_scenes; }
	float					fps					();
	VulkanRender&			render				() { return m_render; }
	void*					windowHandle		() { return m_window.getHandle(); }

#ifdef SCL_APPLE
	void					tick				();
	void					draw				();
	void					onTouchBegin		(const uint16 touchID, const float x, const float y) { currentUI().onTouchBegin (touchID, x, y); }
	void					onTouchEnd			(const uint16 touchID, const float x, const float y) { currentUI().onTouchEnd   (touchID, x, y); }
    void					onTouchMove			(const uint16 touchID, const float x, const float y) { currentUI().onTouchMove  (touchID, x, y); }
	void					onChar				(wchar_t c) { currentUI().onChar(c); }
    void					resize				(const int width, const int height);
	bool					isEditing			(const char** controlText) const;
#endif

#ifdef SCL_WIN
	void					run					();

	static bool				staticOnEvent		(void* caller, void* hWnd, unsigned int message, unsigned int wParam, unsigned int lParam) { return static_cast<Client*>(caller)->onEvent(hWnd, message, wParam, lParam); }  
	bool					onEvent				(void* hWnd, unsigned int message, unsigned int wParam, unsigned int lParam);
#endif

private:
	//void					_initIMGUI			();
	void					_onGUI				();
	void					_renderScene		(uint64 diff);
	//void					_onGUIScene			(const int sceneIndex, bool& isContextMenuOpen);
	//void					_onGUIObject		(Object* const object, bool& isContextMenuOpen);
	//void					_onGUIProperty		(Object* const object);

private:

#ifdef TEST_VULKAN
	VulkanRender			m_render;
#else
	UIRenderOpenGL			m_render;
#endif
	

#ifdef SCL_WIN
	gfx::Win32Window		m_window;

	bool					m_dragging;
	scl::vector2i			m_dragPrev;

	bool					m_rightDragging;
	scl::vector2i			m_rightDragPrev;
#endif

	Env*					m_env;

	Camera*					m_camera;

	// for object test 	(读取gltf file data解析到自己的内存中，然后渲染)
	Object*					m_object;
	Primitive*				m_gridPrimitive;
	Primitive*				m_testVulkanPrimitive;
	Primitive*				m_testVulkanPrimitiveColor;
	Object*					m_simpleAnimation;
	Primitive*				m_bonePrimitive;

	// for direct render test 	(直接渲染 gltf file data,没有解析过程)
	Material*				m_material;
	cgltf_data*				m_gltf;
	void*					m_gltfRenderData;
	int						m_totalFrame;
	uint64					m_totalTime;


	scl::varray<Scene*>		m_scenes;
	scl::varray<Animation*>	m_animations;

	Config					m_config;
	MainGUI					m_gui;

	//Object*					m_selectObject;
};

} //namespace cat



