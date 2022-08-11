
/*
	TODO
	2019.06.25 
		primitive �Ѿ�������Ⱦ��
		BufferMap Ӧ�����ĵ����⻹û���
		���� gltf load ��  normal��tangent û�д������shader����û�У�Ӧ��˵��һ�� 
		
			
*/

#pragma once

#include "scl/type.h"
#include "scl/vector.h"

#include "cat/uiRenderOpenGL.h"
#include "catVulkanRender/vulkanRender.h"

#include "cat/def.h"

#include "./config.h"

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

class Client : public gfx::IEventHandler
{
public:
	Client();
	virtual ~Client();

	static Client&			inst			();
	void					init			(const int width, const int height);

	void					loadGltf		(const char* const filename);
	void					updateAnimation	(const double diff);
	Object*					findObject		(const char* const objectName);
	Object*					getObject		(int sceneIndex, int index);

#ifdef SCL_APPLE
	void					tick			();
	void					draw			();
	void					onTouchBegin	(const uint16 touchID, const float x, const float y) { currentUI().onTouchBegin (touchID, x, y); }
	void					onTouchEnd		(const uint16 touchID, const float x, const float y) { currentUI().onTouchEnd   (touchID, x, y); }
    void					onTouchMove     (const uint16 touchID, const float x, const float y) { currentUI().onTouchMove  (touchID, x, y); }
	void					onChar			(wchar_t c) { currentUI().onChar(c); }
    void					resize			(const int width, const int height);
	bool					isEditing		(const char** controlText) const;
#endif

#ifdef SCL_WIN
	void					run				();

	// implement gfx::IEventHandler
	virtual bool			onEvent			(void* hWnd, unsigned int message, unsigned int wParam, unsigned int lParam);
#endif

private:
	void					_initIMGUI		();
	void					_onGUI			();
	void					_renderScene	(uint64 diff);

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
#endif

	Env*					m_env;

	Camera*					m_camera;

	// for object test 	(��ȡgltf file data�������Լ����ڴ��У�Ȼ����Ⱦ)
	Object*					m_object;
	Primitive*				m_gridPrimitive;
	Primitive*				m_testVulkanPrimitive;
	Primitive*				m_testVulkanPrimitiveColor;
	Object*					m_simpleAnimation;
	Primitive*				m_bonePrimitive;

	// for direct render test 	(ֱ����Ⱦ gltf file data,û�н�������)
	Material*				m_material;
	cgltf_data*				m_gltf;
	void*					m_gltfRenderData;
	int						m_totalFrame;
	uint64					m_totalTime;

	scl::varray<Scene*>		m_scenes;
	scl::varray<Animation*>	m_animations;

	Config					m_config;
};

} //namespace cat



