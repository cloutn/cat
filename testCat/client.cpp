#include "./client.h"

#include "./testPrimitive.h"
#include "./simpleShape.h"

#include "cat/camera.h"
#include "cat/object.h"
#include "cat/material.h"
#include "cat/animation.h"
#include "cat/shader.h"
#include "cat/primitive.h"
#include "cat/gltfLoader.h"
#include "cat/scene.h"
#include "cat/env.h"
#include "cat/def.h"
#include "cat/vertex.h"
#include "cat/terrain.h"

#include "scl/type.h"
#include "scl/time.h"
#include "scl/log.h"
#include "scl/vector.h"
#include "scl/file.h"

#ifdef SCL_WIN
#include <Windows.h>
#endif

namespace cat {

using scl::matrix;
using scl::vector2;
using scl::vector3;
using scl::vector4;
using scl::string64;


inline bool Keydown	(int vKey) { return (GetAsyncKeyState(vKey) & 0x8000) ? 1 : 0; }
inline bool Keyup	(int vKey) { return (GetAsyncKeyState(vKey) & 0x8000) ? 0 : 1; }

Client::Client()
{
#ifdef SCL_WIN
	m_dragging					= false;
	m_dragPrev					= { 0, 0 };
	m_rightDragging				= false;
	m_rightDragPrev				= { 0, 0 };
#endif

	//m_gridPrimitive				= NULL;
	m_bonePrimitive				= NULL;
	m_grid						= NULL;

	m_env						= NULL;
	m_object					= NULL;
	m_camera					= new Camera();
	m_selectObject				= NULL;
	m_totalFrame				= 0;
	m_totalTime					= 1;
	m_operateType				= OPERATE_TYPE_TRANSLATE;
	m_transformType				= TRANSFORM_TYPE_LOCAL;
	m_terrain					= new Terrain();

	m_mousePosition.clear();
}

void Client::init()
{
	m_config.load("config.yaml");

#ifdef SCL_WIN
	m_window.init(m_config.screenPos.x, m_config.screenPos.y, m_config.screenSize.x, m_config.screenSize.y, L"main", L"", true);
	m_render.init(m_window.getInstance(), m_window.getHandle());
	m_render.setOnSurfaceResize(scl::bind(this, &Client::OnSurfaceResize));
	m_render.setReverseZ(m_config.reverseZ);
	m_window.registerEventHandler(scl::bind(this, &Client::onEvent));
#endif

	m_env = new Env();
	m_env->setRender(&m_render);
	m_env->setDefaultShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag");
	m_env->setDefaultMaterial("art/default.png");

	m_camera->set(
		{0, 0, 2}, 
		{0, 0, -1}, 
		{0, 1, 0}, 
		45.f, 
		static_cast<float>(m_render.getDeviceWidth())/m_render.getDeviceHeight(), 
		0.1f, 
		100.f, 
		m_config.reverseZ ? scl::z_range::one_to_zero : scl::z_range::zero_to_one);

	//m_camera->setOrtho(true);

	m_gui.init(this);
	m_gui.registerEvent(GUI_EVENT_PICK_PASS_CLICK, scl::bind(this, &Client::OnButtonClick_PickPass));

	loadGltf("art/SimpleSkin/SimpleSkin.gltf");
	loadGltf("art/chibi_idle/scene.gltf");

	m_object = findObject("RootNode (gltf orientation matrix)");
	m_bonePrimitive = createBone(m_object, &m_render, m_env);

	testPrimitive_vertexAttr(&m_render, m_env);
	testPrimitive_vertexPosition(&m_render, m_env);
	testPrimitive_vertexPosition_edgeCase(&m_render, m_env);

	//m_gridPrimitive = _createGridPrimitive(&m_render, m_env);
	m_grid = _createGrid(&m_render, m_env);

	m_terrain->init(&m_render, m_env);

	for (int i = 0; i < m_scenes.size(); ++i)
	{
		string64 fn;
		fn.format("scene_%d.yaml", i);
		m_scenes[i]->save(fn.c_str());
	}
	//m_config.save("config.yaml");
}

void Client::loadGltf(const char* const filename)
{
	if (NULL == m_env)
	{
		assert(false);
		return;
	}

	GltfLoader loader;
	loader.loadFile(filename, &m_render, m_env, m_scenes, m_animations);
}

Client::~Client()
{
	// 这里就是 Client 的显式 shutdown。Client 在 main 内 new/delete，dtor 触发时机完全可控；
	// 不再用 Meyer's singleton（已删除 inst()），避免 g_client 跨 main 边界静态析构与其它全局相互依赖。
	//
	// 清理顺序约束（一旦改动需同步更新这个注释）：
	//   1. m_gui.release()          先关 UI，避免后续帧回调访问已析构的对象
	//   2. m_animations / m_scenes  场景层：依赖 m_env 中的 Shader/Material 仍存活
	//   3. m_terrain / m_grid / m_bonePrimitive / m_camera  渲染辅助对象，同样依赖 m_env + m_render
	//   4. m_env                    内含 ShaderCache，其析构会通过 Shader::~Shader 调 m_render->releaseShader()
	//                               所以必须在 m_render 真正销毁之前 delete m_env
	//   5. Object::releaseObjectIDMap()  最后清全局 ID map（此时已无 Object 残留）
	//   6. ~VulkanRender 在 Client 所有成员析构后由 m_render 自动析构（含 vkDeviceWaitIdle）
	//
	// 注意：m_render.release() 当前实现是空函数；真正的 GPU 资源清理在 ~VulkanRender，
	// ShaderCache 必须在 m_env 析构内、~VulkanRender 之前完成 releaseShader 调用，靠的就是
	// "先 delete m_env，最后才让 m_render 自然析构" 这个顺序，不要改。
	m_config.save("config.yaml");

	m_gui.release();

	for (int i = 0; i < m_animations.size(); ++i)
		delete m_animations[i];

	for (int i = 0; i < m_scenes.size(); ++i)
		delete m_scenes[i];

	delete m_terrain;
	delete m_camera;
	m_render.release();	
	delete m_grid;
	//delete m_gridPrimitive;
	delete m_bonePrimitive;
	delete m_env;
	Object::releaseObjectIDMap();

	// scl::log::release() 由 main() 末尾统一调用一次；这里不再 release，避免双重 release
}

void Client::_renderScene(bool isPick)
{
	m_terrain->draw(m_camera->matrix(), isPick);

	for (int i = 0; i < m_scenes.size(); ++i)
	{
		m_scenes[i]->draw(m_camera->matrix(), isPick);
	}

	//if (NULL != m_gridPrimitive)
	//	m_gridPrimitive->draw(m_camera->matrix(), NULL, 0, isPick);
	if (NULL != m_grid)
		m_grid->draw(m_camera->matrix(), isPick);

	if (NULL != m_bonePrimitive)
	{
		scl::varray<vertex_color> vertices;
		scl::varray<uint16> indices;
		collectBoneVertices(m_object, vertices, indices);
		m_bonePrimitive->updateVertices(vertices.begin(), vertices.size(), sizeof(vertex_color));
		m_bonePrimitive->draw(m_camera->matrix(), NULL, 0, isPick);
	}
}

void Client::_processKeydown()
{
	if (m_gui.wantCaptureKeyboard())
		return;
	if (!m_window.IsForegroundWindow())
		return;

	if (Keydown(VK_RBUTTON))
	{
		float speed = 0.25f;
		if (Keydown('W'))
			m_camera->move_front(speed);
		if (Keydown('S'))
			m_camera->move_front(-speed);
		if (Keydown('A'))
			m_camera->move_side(-speed);
		if (Keydown('D'))
			m_camera->move_side(speed);
	}
	else
	{
		if (Keydown('W'))
			m_operateType = OPERATE_TYPE_TRANSLATE;
		if (Keydown('E'))
			m_operateType = OPERATE_TYPE_ROTATE;
		if (Keydown('R') && transformType() == TRANSFORM_TYPE_LOCAL)
			m_operateType = OPERATE_TYPE_SCALE;
	}
}

void Client::_clickSelectObject(int x, int y)
{
	m_env->clearPickPrimtives();

	m_render.beginPickPass(scl::vector4{1, 1, 1, 1});

	_renderScene(true);

	scl::vector4 pickColor = m_render.endPickPass(x, y);
	Primitive* primitive = m_env->getPickPrimitive(pickColor);
	if (NULL != primitive)
	{
		m_selectObject = primitive->parentObject();
		m_gui.setForceOpenSceneTree(true);
		printf("picked object = %llx\n", reinterpret_cast<uint64>(primitive));	
	}
}

bool Client::OnButtonClick_PickPass(GUIEvent&)
{
	_clickSelectObject(640, 480);
	return true;
}

void Client::OnSurfaceResize(int width, int height)
{
	if (width > 0 && height > 0)
	{
		m_camera->setAspect(static_cast<float>(width) / height);
	}
}

#ifdef SCL_WIN
void Client::run()
{
	uint64 lastTick = SCL_TICK;

	while (m_window.run())
	{
		const uint64 now = SCL_TICK;
		uint64 diff = now - lastTick;
		lastTick = now;

		_processKeydown();

		m_gui.onGUI();

		updateAnimation(static_cast<double>(diff));

		m_render.clear();

#ifdef TEST_VULKAN

		m_render.beginDraw();

		m_render.beginScenePass(m_config.getClearColorf());
		_renderScene(false);
		m_gui.Render();
		m_render.endScenePass();

		m_render.endDraw();
#else
		m_gridPrimitive->draw(m_camera->matrix(), NULL, 0, false);

		//m_object->draw(m_camera->matrix(), false);
#endif

		m_render.swap();

		++m_totalFrame;
		m_totalTime += diff;
		if (m_totalTime > 10 * 1000)
		{
			m_totalFrame = 1;
			m_totalTime = diff;
		}

		//if (m_totalFrame % 100 == 0)
		//{
		//	printf("pos = %d, %d, size = %d, %d\n", m_window.getPositionX(), m_window.getPositionY(), m_window.getWidth(), m_window.getHeight());
		//}

		//scl::usleep(1);
	}

#ifdef TEST_VULKAN
	m_render.waitIdle();	
#endif

}
#endif


#ifdef SCL_WIN

bool Client::onEvent(void* hWnd, uint32_t message, intptr_t wParam, intptr_t lParam)
{
	m_gui.onEvent(hWnd, message, wParam, lParam);
	bool wantCaptureMouse		= m_gui.wantCaptureMouse();
	bool wantCaptureKeyboard	= m_gui.wantCaptureKeyboard();

	switch (message)
	{
	case WM_LBUTTONDOWN:
		{
			if (wantCaptureMouse)
				break;

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			
			m_dragging = true;
			m_dragPrev.set(x, y);

		}
		break;
	case WM_LBUTTONUP:
		{
			if (wantCaptureMouse)
				break;

			m_dragging = false;
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			_clickSelectObject(x, y);
		}
		break;
	case WM_RBUTTONDOWN:
		{
			if (wantCaptureMouse)
				break;

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			
			m_rightDragging = true;
			m_rightDragPrev.set(x, y);
		}
		break;
	case WM_RBUTTONUP:
		{
			if (wantCaptureMouse)
				break;

			m_rightDragging = false;
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
		}
		break;
	case WM_MOUSEMOVE:
		{
			if (wantCaptureMouse)
				break;

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_mousePosition.set(x, y);
			
			if (m_dragging)
			{

			}
			else if (m_rightDragging)
			{
				int dx = x - m_rightDragPrev.x;
				int dy = y - m_rightDragPrev.y;
				float speed = 0.3f;
				//printf("x = %d, prev_x = %d, y = %d, prev_y = %d, dx = %d, dy = %d\n", x, m_rightDragPrev.x, y, m_rightDragPrev.y, dx, dy);
				//m_camera->move({dx / 1000.f, dy / 1000.f, 0});
				//m_camera->rotate(-dy, -dx, 0);
				m_camera->orbit_right(-dy * speed);
				m_camera->orbit_up(-dx * speed);

				m_rightDragPrev.set(x, y);
			}
		}
		break;
	case WM_CHAR:
		{
			if (wantCaptureKeyboard)
				break;

			wchar c = wParam;
		}
		break;
	case WM_KEYDOWN:
		{
			if (wantCaptureKeyboard)
				break;

			//uint32 keyCode = wParam;
			//char s[2] = { 0 };
			//s[0] = (char)keyCode;
			//float speed = 1.0f;
			//if (keyCode == 'W')
			//{
			//	m_camera->move(0, 0, -speed);
			//}
			//else if (keyCode == 'S')
			//{
			//	m_camera->move(0, 0, speed);
			//}
			//else if (keyCode == 'A')
			//{
			//	m_camera->move(-speed, 0, 0);
			//}
			//else if (keyCode == 'D')
			//{
			//	m_camera->move(speed, 0, 0);
			//}
		}
		break;
	case WM_SIZE:
		{
			int width	= LOWORD(lParam);
			int height	= HIWORD(lParam);
			m_config.screenSize.set(width, height);
			//printf("WM_SIZE : width = %d, height = %d\n", width, height);
		}
		break;
	case WM_MOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_config.screenPos.set(x, y);
			//printf("WM_MOVE : x = %d, y = %d\n", x, y);
		}
		break;
	default:
		{
			return false;
		}
		break;
	}; // switch



	// imgui need DefWindowProc, so we must return false to call DefWindowProc in win32Window.cpp
	if (wantCaptureMouse || wantCaptureKeyboard)
		return false;

	return true;
}
#endif

void Client::updateAnimation(const double diff)
{
	for (int i = 0; i < m_animations.size(); ++i)
	{
		Animation* anim = m_animations[i];
		anim->update(diff);

		////// TODO DEBUG ONLY!!! //////
		///break;
		////// TODO DEBUG ONLY!!! //////
	}
}

Object* Client::findObject(const char* const objectName)
{
	for (int i = 0; i < m_scenes.size(); ++i)
	{
		Object* object = m_scenes[i]->objectByName(objectName);
		if (NULL != object)
			return object;
	}
	return NULL;
}

Object* Client::getObject(int sceneIndex, int objectIndex)
{
	if (sceneIndex >= m_scenes.size())
		return NULL;
	Scene* scene = m_scenes[sceneIndex];
	if (objectIndex >= scene->objectCount())
		return NULL;
	return m_scenes[sceneIndex]->object(objectIndex);
}

float Client::fps()
{
	return (m_totalTime == 0) ? 0 : m_totalFrame / (m_totalTime / 1000.f);
}

int Client::getScreenWidth() const
{
	return m_render.getDeviceWidth();
}

int Client::getScreenHeight() const
{
	return m_render.getDeviceHeight();
}

void Client::setSelectObject(Object* object)
{
	m_selectObject = object;
}

cat::Object* Client::getSelectObject()
{
	return m_selectObject;
}

int Client::getSelectObjectID() const
{
	if (NULL == m_selectObject)
		return -1;
	return m_selectObject->id();
}

} //namespace cat



