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

Primitive* _createGridPrimitive				(IRender* render, Env* env);
Primitive* _createTestVulkanPrimitive		(IRender* render, Env* env);
Primitive* _createTestVulkanPrimitiveColor	(IRender* render, Env* env);
Primitive* _createBone(Object* root, IRender* render, Env* env);

Client::Client()
{
#ifdef SCL_WIN
	m_dragging					= false;
	m_dragPrev					= { 0, 0 };
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
}



void Client::init(const int width, const int height)
{
#ifdef SCL_WIN
	m_window.init(width, height, L"main", L"", true);
	m_render.init(m_window.getInstance(), m_window.getHandle(), m_config.clearColor);
	m_window.registerEventHandler(*this);
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
		{ 0, 3, VertexAttr::DATA_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
		{ 1, 4, VertexAttr::DATA_TYPE_UNSIGNED_BYTE, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
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
	p->setIndices		(indices, countof(indices), VertexAttr::DATA_TYPE_UNSIGNED_SHORT);
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
		{ 0, 3, VertexAttr::DATA_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
		{ 5, 4, VertexAttr::DATA_TYPE_UNSIGNED_BYTE, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
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
	p->setIndices(indices, countof(indices), VertexAttr::DATA_TYPE_UNSIGNED_SHORT);
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
		{ 0, 3, VertexAttr::DATA_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
		{ 5, 4, VertexAttr::DATA_TYPE_UNSIGNED_BYTE, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
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
	p->setIndices(indices.begin(), indices.size(), VertexAttr::DATA_TYPE_UNSIGNED_SHORT);
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
		{ 0, 4, VertexAttr::DATA_TYPE_FLOAT,		0, sizeof(vertex_coord), 0 },
		{ 1, 4, VertexAttr::DATA_TYPE_UNSIGNED_BYTE, 1, sizeof(vertex_coord), OFFSET(vertex_coord, color) },
		{ 2, 2, VertexAttr::DATA_TYPE_FLOAT,		0, sizeof(vertex_coord), OFFSET(vertex_coord, texcoord) }
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
	p->setIndices(indices, countof(indices), VertexAttr::DATA_TYPE_UNSIGNED_SHORT);
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
		{ 0, 3, VertexAttr::DATA_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
		{ 1, 4, VertexAttr::DATA_TYPE_UNSIGNED_BYTE, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
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
	p->setIndices(indices, countof(indices), VertexAttr::DATA_TYPE_UNSIGNED_SHORT);
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
	if (io.WantCaptureMouse || io.WantCaptureKeyboard)
		return false;

	switch (message)
	{
	case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			
			m_dragging = true;
			m_dragPrev.set(x, y);
		}
		break;
	case WM_LBUTTONUP:
		{
			m_dragging = false;
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
		}
		break;
	case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			
			if (m_dragging)
			{
				int dx = x - m_dragPrev.x;
				int dy = y - m_dragPrev.y;
				m_camera->move({dx / 1000.f, dy / 1000.f, 0});
				m_dragPrev.set(x, y);
			}
		}
		break;
	case WM_CHAR:
		{
			wchar c = wParam;
		}
		break;
	case WM_KEYDOWN:
		{
			uint32 keyCode = wParam;
			char s[2] = { 0 };
			s[0] = (char)keyCode;
			float speed = 1.0f;
			if (keyCode == 'W')
			{
				m_camera->move(0, 0, -speed);
			}
			else if (keyCode == 'S')
			{
				m_camera->move(0, 0, speed);
			}
			else if (keyCode == 'A')
			{
				m_camera->move(-speed, 0, 0);
			}
			else if (keyCode == 'D')
			{
				m_camera->move(speed, 0, 0);
			}
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
	ImFont* myshFont = io.Fonts->AddFontFromFileTTF(fontFilename.c_str(), 24.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

	fontFilename = fontsDir;
	fontFilename += "ArialUni.ttf";
	ImFont* font = io.Fonts->AddFontFromFileTTF(fontFilename.c_str(), 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	IM_ASSERT(font != NULL);

	io.FontDefault = myshFont;

	m_render.initIMGUI();
}



} //namespace cat



