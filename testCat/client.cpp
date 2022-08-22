#include "./client.h"

#include "cat/camera.h"
#include "cat/object.h"
#include "cat/material.h"
#include "cat/animation.h"
#include "cat/shader.h"
#include "cat/primitive.h"
#include "cat/gltf_raw_render.h"
#include "cat/scene.h"
#include "cat/env.h"
#include "cat/def.h"

#include "gfx/vertex.h"

#include "scl/type.h"
#include "scl/time.h"
#include "scl/log.h"
#include "scl/vector.h"
#include "scl/file.h"

#include "imgui_impl_win32.h"
#include "cgltf/cgltf.h"

#ifdef SCL_WIN
#include <Windows.h>
#endif

#ifdef SCL_WIN
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

namespace cat {

using scl::matrix;
using scl::vector2;
using scl::vector3;
using scl::vector4;
using scl::file;

namespace ui = ImGui;

Primitive* _createGridPrimitive				(IRender* render, Env* env);
Primitive* _createTestVulkanPrimitive		(IRender* render, Env* env);
Primitive* _createTestVulkanPrimitiveColor	(IRender* render, Env* env);
Primitive* _createBone(Object* root, IRender* render, Env* env);

inline bool Keydown	(int vKey) { return (GetAsyncKeyState(vKey) & 0x8000) ? 1 : 0; }
inline bool Keyup	(int vKey) { return (GetAsyncKeyState(vKey) & 0x8000) ? 0 : 1; }
//#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
//#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)
//#define KEY_STATE(vk_code) (GetKeyState(vk_code) & 0x8000 ? 1 : 0)



Client::Client()
{
#ifdef SCL_WIN
	m_dragging					= false;
	m_dragPrev					= { 0, 0 };
	m_rightDragging				= false;
	m_rightDragPrev				= { 0, 0 };
#endif
	m_simpleAnimation			= new Object(NULL);

	m_gridPrimitive				= NULL;
	m_testVulkanPrimitive		= NULL;
	m_testVulkanPrimitiveColor	= NULL;
	m_bonePrimitive				= NULL;

	m_camera					= new Camera();
	m_material					= new Material();
	m_gltf						= NULL;
	m_gltfRenderData			= NULL;
	m_totalFrame				= 0;
	m_totalTime					= 1;

	m_selectObject				= NULL;
}



void Client::init(const int width, const int height)
{
#ifdef SCL_WIN
	m_window.init(width, height, L"main", L"", true);
	m_render.init(m_window.getInstance(), m_window.getHandle(), m_config.clearColor);
	m_window.registerEventHandler(this, Client::staticOnEvent);
#endif

	m_env = new Env();
	m_env->setRender(&m_render);
	m_env->setDefaultShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag");
	m_env->setDefaultMaterial("art/default.png");

	m_camera->set({0, 0, 0}, {0, 0, -1}, {0, 1, 0}, 45.f, static_cast<float>(m_render.getDeviceWidth())/m_render.getDeviceHeight(), 0.1f, 100.f);

	_initIMGUI();

	///m_gltf = gltf_load_from_file("black_kizuna/out2.gltf");
	///m_gltf = gltf_load_from_file("chibi_idle/scene.gltf");
	///m_gltfRenderData = gltf_create_render_data();

	loadGltf("art/chibi_idle/scene.gltf");
	loadGltf("art/SimpleSkin/SimpleSkin.gltf");

	m_object = findObject("RootNode (gltf orientation matrix)");
	m_bonePrimitive = _createBone(m_object, &m_render, m_env);

	m_gridPrimitive = _createGridPrimitive(&m_render, m_env);
}

void Client::loadGltf(const char* const filename)
{
	if (NULL == m_env)
	{
		assert(false);
		return;
	}
	m_env->clearGltfNodeMap();	

	string256 path = filename;
	scl::extract_path(path.pstring());

	cgltf_data* data = gltf_load_from_file(filename);
	if (NULL == data)
		return;
	if (NULL == data->scene && data->scenes_count == 0)
		return;

	for (int sceneIndex = 0; sceneIndex < static_cast<int>(data->scenes_count); ++sceneIndex)
	{
		cgltf_scene& sceneNode = data->scenes[sceneIndex];
		Scene* scene = new Scene();
		scene->load(sceneNode, path.c_str(), m_env);
		m_scenes.push_back(scene);
	}

	m_animations.reserve(data->animations_count);
	for (cgltf_size i = 0; i < data->animations_count; ++i)
	{
		cgltf_animation&	animationNode	= data->animations[i];
		Animation*			animation		= new Animation();
		animation->load(animationNode, m_env);
		m_animations.push_back(animation);
	}

	cgltf_free(data);
}

Client::~Client()
{
	m_render.releaseIMGUI();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	for (int i = 0; i < m_animations.size(); ++i)
		delete m_animations[i];

	for (int i = 0; i < m_scenes.size(); ++i)
		delete m_scenes[i];

	delete m_simpleAnimation;
	gltf_release(m_gltf, m_gltfRenderData);
	delete m_camera;
	delete m_material;
	m_render.release();	
	scl::log::release();
	delete m_gridPrimitive;
	delete m_bonePrimitive;
	delete m_testVulkanPrimitive;
	delete m_testVulkanPrimitiveColor;
	delete m_env;
	Object::releaseObjectIDMap();
}

Client& Client::inst()
{
	static Client g_client;
	return g_client;
}

class vertex_color
{
public:
	vector3 position;
	uint32	color;
};

class vertex_coord
{
public:
	vector4 position;
	uint32	color;
	vector2 texcoord;
};

void _loadPrimivteFromMemory2(Primitive* p, IRender* render, Env* env)
{
	// index
	uint16 indices[] = { 0, 1, 2, 3 };
	// attr
	VertexAttr attrs[] = {
		{ 0, 3, VERTEX_DATA_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
		{ 1, 4, VERTEX_DATA_TYPE_UNSIGNED_BYTE, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	};
	int attrBuffers[] = { 0, 0 };
	ShaderMacro macros[1] = { {"COLOR", ""} };
	// vertex 
	uint32 color = 0xCCCCCCFF;
	vertex_color vertices[] = {
		{ vector3{ -100, 0, 0 }, color },
		{ vector3{ 100, 0, 0 }, color },
		{ vector3{ -100, 1, 0 }, color },
		{ vector3{ 100, 1, 0 }, color }
	};

	p->setEnv			(env);
	p->setRender		(render);
	p->setPrimitiveType	(PRIMITIVE_TYPE_LINES);
	p->setIndices		(indices, countof(indices), VERTEX_DATA_TYPE_UNSIGNED_SHORT);
	p->setAttrs			(attrs, countof(attrs), attrBuffers);
	p->setVertices		(vertices, countof(vertices), sizeof(vertex_color));
	p->setTexture		(NULL);
	p->loadShader		(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros, 1);
}


Primitive* _createGridPrimitive(IRender* render, Env* env)
{
	Primitive* p = new Primitive();
	// attr
	VertexAttr attrs[] = {
		{ 0, 3, VERTEX_DATA_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
		{ 5, 4, VERTEX_DATA_TYPE_UNSIGNED_BYTE, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	};
	int attrBuffers[] = { 0, 0 };
	ShaderMacro macros[1];
	macros[0].name = "COLOR";

	// vertex 
	const int ROW		= 3;
	const int COLUMN	= 3;
	const int VERTEX_COUNT = (ROW + COLUMN) * 2;
	uint32 color = 0xFFFF0000;
	vertex_color vertices[VERTEX_COUNT];
	memset(vertices, 0, sizeof(vertices));
	for (int i = 0; i < ROW; ++i)
	{
		float z = static_cast<float>(i - (ROW - 1) / 2);	
		vertices[2 * i		] = { vector3{-10.0f,	0, z}, color };
		vertices[2 * i + 1	] = { vector3{10.0f,	0, z}, color };
	}
	for (int i = 0; i < COLUMN; ++i)
	{
		float x = static_cast<float>(i - (COLUMN- 1) / 2);	
		vertices[ROW * 2 + 2 * i		] = { vector3{x,	0, 10}, color };
		vertices[ROW * 2 + 2 * i + 1	] = { vector3{x,	0, -10}, color };

	}

	// index
	uint16 indices[VERTEX_COUNT];
	for (int i = 0; i < VERTEX_COUNT; ++i)
		indices[i] = i;

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_LINES);
	p->setIndices(indices, countof(indices), VERTEX_DATA_TYPE_UNSIGNED_SHORT);
	p->setAttrs(attrs, countof(attrs), attrBuffers);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_color));
	p->setTexture(NULL);
	p->loadShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros, 1);
	return p;
}

void _collectVertices(Object* root, scl::varray<vertex_color>& vertices, scl::varray<uint16>& indices, int level = 0)
{
	if (root->childCount() <= 0)
		return;

	matrix matRoot = root->globalMatrix();
	vector3 posRoot { 0, 0, 0};
	posRoot.mul_matrix(matRoot);
	uint fromColor = 0xFFFFFFFF;
	uint toColor = 0xFF00FF00;
	vertices.push_back({ posRoot, fromColor});
	int rootIndex = vertices.size() - 1;
	for (int i = 0; i < root->childCount(); ++i)
	{
		Object* obj = root->child(i);
		scl::matrix m = obj->globalMatrix();
		vector3 pos = { 0, 0, 0 };
		pos.mul_matrix(m);
		vertices.push_back({ pos, toColor });
		indices.push_back(rootIndex);
		indices.push_back(vertices.size() - 1);
	}

	for (int i = 0; i < root->childCount(); ++i)
	{
		Object* obj = root->child(i);
		_collectVertices(obj, vertices, indices, ++level);
	}
}

Primitive* _createBone(Object* root, IRender* render, Env* env)
{
	if (NULL == root)
		return NULL;
	Primitive* p = new Primitive();
	// attr
	VertexAttr attrs[] = {
		{ 0, 3, VERTEX_DATA_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
		{ 5, 4, VERTEX_DATA_TYPE_UNSIGNED_BYTE, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	};
	int attrBuffers[] = { 0, 0 };
	ShaderMacro macros[1];
	macros[0].name = "COLOR";

	scl::varray<vertex_color> vertices;
	scl::varray<uint16> indices;
	_collectVertices(root, vertices, indices);
	if (indices.size() == 0)
	{
		delete p;
		return NULL;
	}

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_LINES);
	p->setIndices(indices.begin(), indices.size(), VERTEX_DATA_TYPE_UNSIGNED_SHORT);
	p->setAttrs(attrs, countof(attrs), attrBuffers);
	p->setVertices(vertices.begin(), vertices.size(), sizeof(vertex_color));
	p->setTexture(NULL);
	p->loadShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros, 1);
	return p;
}

Primitive* _createTestVulkanPrimitive(IRender* render, Env* env)
{
	Primitive* p = new Primitive();
	// attr
	VertexAttr attrs[] = {
		{ 0, 4, VERTEX_DATA_TYPE_FLOAT,		0, sizeof(vertex_coord), 0 },
		{ 1, 4, VERTEX_DATA_TYPE_UNSIGNED_BYTE, 1, sizeof(vertex_coord), OFFSET(vertex_coord, color) },
		{ 2, 2, VERTEX_DATA_TYPE_FLOAT,		0, sizeof(vertex_coord), OFFSET(vertex_coord, texcoord) }
	};
	int attrBuffers[] = { 0, 0, 0 };
	ShaderMacro macros[1] = { {"USE_COLOR", ""} };

	// vertex 
	vertex_coord vertices[3] = 
	{
		0,		0,		-1, 1,	0xFF0000FF, 0, 0,
		0.5,	0,		-1, 1,	0x00FF00FF, 0, 1,
		0,		0.5,	-1, 1,	0x0000FFFF, 1, 0,
	};

	// index
	uint16 indices[] = { 0, 1, 2 };

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_TRIANGLES);
	p->setIndices(indices, countof(indices), VERTEX_DATA_TYPE_UNSIGNED_SHORT);
	p->setAttrs(attrs, countof(attrs), attrBuffers);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_coord));
	p->setTexture(NULL);
	p->loadShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros, 1);
	return p;
}


Primitive* _createTestVulkanPrimitiveColor(IRender* render, Env* env)
{
	Primitive* p = new Primitive();

	VertexAttr attrs[] = {
		{ 0, 3, VERTEX_DATA_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
		{ 1, 4, VERTEX_DATA_TYPE_UNSIGNED_BYTE, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	};
	int attrBuffers[] = { 0, 0 };
	ShaderMacro macros[1] = { {"COLOR", ""} };

	// vertex 
	vertex_color vertices[3] = 
	{
		0,		0,		-1, 0xFF0000FF,
		1,		0,		-1,	0x00FF00FF,
		0,		1,		-1, 0x0000FFFF,
	};

	// index
	uint16 indices[] = { 0, 1, 2 };

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_TRIANGLES);
	p->setIndices(indices, countof(indices), VERTEX_DATA_TYPE_UNSIGNED_SHORT);
	p->setAttrs(attrs, countof(attrs), attrBuffers);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_color));
	p->setTexture(NULL);
	p->loadShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros, 1);
	return p;
}


void Client::_onGUI()
{
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (m_config.showDemoWindow)
		ImGui::ShowDemoWindow(&m_config.showDemoWindow);

	ImGui::Begin("Scene");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	//ImGui::Text("Hello from another window!");
	for (int i = 0; i < m_scenes.size(); ++i)
	{
		_onGUIScene(i);
	}
	//if (ImGui::Button("Close Me"))
	//	m_config.showAnotherWindow = false;
	ImGui::End();

	_onGUIProperty(m_selectObject);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Debug main");                          // Create a window called "Hello, world!" and append into it.

		string256 fpsStr;
		float fps = (m_totalTime == 0) ? 0 : m_totalFrame / (m_totalTime / 1000.f);
		fpsStr.format("fps = %.2f", fps);

		ImGui::Text(fpsStr.c_str());               // Display some text (you can use a format strings too)

		ImGui::Checkbox("Demo Window", &m_config.showDemoWindow);      // Edit bools storing our window open/close state
		//ImGui::Checkbox("Another Window", &show_another_window);

		//ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		//	counter++;
		//ImGui::SameLine();
		//ImGui::Text("counter = %d", counter);
		if (NULL != m_selectObject)
			ImGui::Text(m_selectObject->name().c_str());               // Display some text (you can use a format strings too)

		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}



	// 3. Show another simple window.
	//if (m_config.showAnotherWindow)
	//{
	//	ImGui::Begin("Another Window", &m_config.showAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	//	ImGui::Text("Hello from another window!");
	//	if (ImGui::Button("Close Me"))
	//		m_config.showAnotherWindow = false;
	//	ImGui::End();
	//}

	// Rendering
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
	if (!is_minimized)
	{
		m_render.drawIMGUI(draw_data);
	}
}

void Client::_renderScene(uint64 diff)
{
	m_render.beginDraw();

	//m_render.bindCommandBuffer();
	updateAnimation(static_cast<double>(diff));
	for (int i = 0; i < m_scenes.size(); ++i)
	{
		m_scenes[i]->draw(m_camera->matrix(), &m_render);
	}

	if (NULL != m_gridPrimitive)
		m_gridPrimitive->draw(m_camera->matrix(), NULL, 0, &m_render);

	if (NULL != m_bonePrimitive)
	{
		scl::varray<vertex_color> vertices;
		scl::varray<uint16> indices;
		_collectVertices(m_object, vertices, indices);
		m_bonePrimitive->updateVertices(vertices.begin(), vertices.size(), sizeof(vertex_color));
		m_bonePrimitive->draw(m_camera->matrix(), NULL, 0, &m_render);
	}

	m_simpleAnimation->draw(m_camera->matrix(), &m_render);

	//m_render.unbindCommandBuffer();

	/// /// imgui
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	_onGUI();


	m_render.endDraw();
}

void Client::_onGUIScene(const int sceneIndex)
{
	Scene* scene = m_scenes[sceneIndex];
	if (NULL == scene)
		return;

	string64 sceneName;
	sceneName.format("Scene_%d", sceneIndex);
	if (ImGui::TreeNode(sceneName.c_str()))
	{
		for (int i = 0; i < scene->objectCount(); ++i)
			_onGUIObject(scene->object(i));

		ImGui::TreePop();
	}
}

void Client::_onGUIObject(Object* const object)
{
	//for (int i = 0; i < 5; i++)
	//{
	//	// Use SetNextItemOpen() so set the default state of a node to be open. We could
	//	// also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
	//	if (i == 0)
	//		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	string128 objectName = object->name().c_str();	
	if (objectName.empty())
		objectName = "[NoName]";

	if (object->childCount() == 0)
	{
		//ui::Text(objectName.c_str());
		int node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
		ImGui::TreeNodeEx(objectName.c_str(), node_flags);
		if (ImGui::IsItemClicked())
		{
			m_selectObject = object;
		}
	}
	else
	{
		ImVec2 buttonSize(200, 0);
		bool nodeOpen = ImGui::TreeNodeEx(objectName.c_str(), 2240);
		if (ImGui::BeginPopupContextItem())
		{
		   // your popup code
			if (ImGui::Button("context menu item 1", buttonSize))
			{
				printf("context menu pressed.");
			}
			if (ImGui::Button("Close", buttonSize))
				ImGui::CloseCurrentPopup();
		   ImGui::EndPopup();
		}
		if (ImGui::IsItemClicked())
		{
			m_selectObject = object;
		}
		if (nodeOpen)
		{
			for (int i = 0; i < object->childCount(); ++i)
			{
				_onGUIObject(object->child(i));
			}
			ImGui::TreePop();
		}
	}
}


//void Client::_renderIMGUI(bool& show_demo_window, bool& show_another_window, scl::vector4& clear_color)
//{
//	// draw imgui
//	//m_render.beginDrawIMGUI();
//	ImGui_ImplWin32_NewFrame();
//	ImGui::NewFrame();
//	_onGUI(show_demo_window, show_another_window, clear_color);
//}

#ifdef SCL_WIN
void Client::run()
{
	uint64 lastTick = SCL_TICK;

	while (m_window.run())
	{
		const uint64 now = SCL_TICK;
		uint64 diff = now - lastTick;
		lastTick = now;

		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureKeyboard)
		{
			float speed = 0.25f;
			if (Keydown('W'))
			{
				//m_camera->move(0, 0, -speed);
				m_camera->move_front(speed);
			}
			if (Keydown('S'))
			{
				m_camera->move_front(-speed);
				//m_camera->move(0, 0, speed);
			}
			if (Keydown('A'))
			{
				m_camera->move_side(-speed);
			}
			if (Keydown('D'))
			{
				m_camera->move_side(speed);
			}
		}


		m_render.clear();

#ifdef TEST_VULKAN

		//_renderIMGUI(showDemoWindow, showAnotherWindow, clearColor);

		_renderScene(diff);

#else
		m_gridPrimitive->draw(m_camera->matrix(), NULL, 0, &m_render);

		m_object->draw(m_camera->matrix(), &m_render);
#endif


		m_render.swap();

		++m_totalFrame;
		m_totalTime += diff;
		if (m_totalTime > 10 * 1000)
		{
			m_totalFrame = 1;
			m_totalTime = diff;
		}

		scl::usleep(10);
	}

#ifdef TEST_VULKAN
	m_render.waitIdle();	
#endif

}
#endif



#ifdef SCL_WIN

bool Client::onEvent(void* hWnd, uint32 message, uint32 wParam, uint32 lParam)
{
	ImGui_ImplWin32_WndProcHandler((HWND)hWnd, message, wParam, lParam);
	ImGuiIO& io = ImGui::GetIO();

	switch (message)
	{
	case WM_LBUTTONDOWN:
		{
			if (io.WantCaptureMouse)
				break;

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			
			m_dragging = true;
			m_dragPrev.set(x, y);
		}
		break;
	case WM_LBUTTONUP:
		{
			if (io.WantCaptureMouse)
				break;

			m_dragging = false;
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
		}
		break;
	case WM_RBUTTONDOWN:
		{
			if (io.WantCaptureMouse)
				break;

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			
			m_rightDragging = true;
			m_rightDragPrev.set(x, y);
		}
		break;
	case WM_RBUTTONUP:
		{
			if (io.WantCaptureMouse)
				break;

			m_rightDragging = false;
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
		}
		break;
	case WM_MOUSEMOVE:
		{
			if (io.WantCaptureMouse)
				break;

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			
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
			if (io.WantCaptureKeyboard)
				break;

			wchar c = wParam;
		}
		break;
	case WM_KEYDOWN:
		{
			if (io.WantCaptureKeyboard)
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
		}
		break;
	default:
		{
			return false;
		}
		break;
	}; // switch



	// imgui need DefWindowProc, so we must return false to call DefWindowProc in win32Window.cpp
	if (io.WantCaptureMouse || io.WantCaptureKeyboard)
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
		Object* object = m_scenes[i]->findObject(objectName);
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

void Client::_initIMGUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(m_window.getHandle());

	string256 envName = "SystemRoot";
	string256 fontsDir;
	GetEnvironmentVariableA(envName.c_str(), fontsDir.c_str(), fontsDir.capacity());
	fontsDir += "\\Fonts\\";

	string256 fontFilename = fontsDir;
	fontFilename += "msyh.ttc";

	io.Fonts->AddFontDefault();
	ImFont* myshFont = NULL;
	float fontSize = 18.0f;
	if (file::exists(fontFilename.c_str()))
	{
		myshFont = io.Fonts->AddFontFromFileTTF(fontFilename.c_str(), fontSize, NULL, io.Fonts->GetGlyphRangesChineseFull());
	}
	else
	{
		printf("Font file not found! %s", fontFilename.c_str());
	}

	fontFilename = fontsDir;
	fontFilename += "ArialUni.ttf";
	if (file::exists(fontFilename.c_str()))
	{ 
		ImFont* font = io.Fonts->AddFontFromFileTTF(fontFilename.c_str(), fontSize, NULL, io.Fonts->GetGlyphRangesChineseFull());
		IM_ASSERT(font != NULL);
	}
	else
	{
		printf("Font file not found! %s", fontFilename.c_str());
	}

	io.FontDefault = myshFont;

	m_render.initIMGUI();
}

#define HZ_CORE_IMGUI_COMPONENT_VAR(func, label, code) ImGui::TextUnformatted(label); ImGui::NextColumn(); ImGui::SetNextItemWidth(-1); if(func) { code } ImGui::NextColumn();

void Client::_onGUIProperty(Object* const object)
{
	if (NULL == object)
		return;

	ImGui::Begin("Property");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

	string128 name = object->name().c_str();
	ImGui::InputText("Name", name.c_str(), name.capacity());
	object->setName(name.c_str());

	//ImGui::BeginColumns("columne1", 2);
	//{
	//	//HZ_CORE_IMGUI_COMPONENT_VAR(ImGui::DragFloat("##Far", &orthoFar), "Far", camera.SetOrthographicFarClip(orthoFar); );

	//	HZ_CORE_IMGUI_COMPONENT_VAR(ImGui::Checkbox("##Fixed Aspect Ratio", &v), "Fixed Aspect Ratio");
	//}
	//ImGui::EndColumns();
	//ImGui::Separator();


	//ImGui::Text("input 2"); 
	//ImGui::SameLine(); 
	//ImGui::Text("input 2"); 


	//ImGui::InputFloat("", &v);
	//if (ImGui::Button("Close Me"))
	//	m_config.showAnotherWindow = false;
	ImGui::End();
}


} //namespace cat



