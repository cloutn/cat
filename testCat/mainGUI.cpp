#include "./mainGUI.h"

#include "./client.h"
#include "./config.h"

#include "cat/object.h"
#include "cat/scene.h"

#include "scl/file.h"

#include "imgui_impl_win32.h"

#ifdef SCL_WIN
#include <Windows.h>
#endif

#ifdef SCL_WIN
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

//#define HZ_CORE_IMGUI_COMPONENT_VAR(func, label, code) ImGui::TextUnformatted(label); ImGui::NextColumn(); ImGui::SetNextItemWidth(-1); if(func) { code } ImGui::NextColumn();

namespace cat {

using scl::file;

MainGUI::MainGUI() : m_client(nullptr), m_selectObject(nullptr)
{

}

void MainGUI::init(Client* client)
{
	m_client = client;
	_initIMGUI();
}


void MainGUI::_initIMGUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(m_client->windowHandle());

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

	m_client->render().initIMGUI();
}


void MainGUI::release()
{
	m_client->render().releaseIMGUI();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void MainGUI::OnGUI()
{
	if (NULL == m_client)
		return;

	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


	Config& config = m_client->config();
	if (config.showDemoWindow)
		ImGui::ShowDemoWindow(&config.showDemoWindow);

	ImGui::Begin("Scene");
	bool isContextMenuOpen = false;
	for (int i = 0; i < m_client->scenes().size(); ++i)
	{
		_onGUIScene(i, isContextMenuOpen);
	}

	ImVec2 buttonSize(200, 0);
	if (!isContextMenuOpen && ImGui::BeginPopupContextWindow())
	{
		// your popup code
		if (ImGui::Button("context menu item 222", buttonSize))
		{
			printf("context menu pressed.");
		}
		if (ImGui::Button("Close", buttonSize))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	ImGui::End();

	_onGUIProperty(m_selectObject);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Debug main");                          // Create a window called "Hello, world!" and append into it.

		string256 fpsStr;
		float fps = m_client->fps();
		fpsStr.format("fps = %.2f", fps);

		ImGui::Text(fpsStr.c_str());               // Display some text (you can use a format strings too)

		ImGui::Checkbox("Demo Window", &config.showDemoWindow);      // Edit bools storing our window open/close state
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
		m_client->render().drawIMGUI(draw_data);
	}
}

void MainGUI::OnEvent(void* hWnd, uint32 message, uint32 wParam, uint32 lParam)
{
	ImGui_ImplWin32_WndProcHandler((HWND)hWnd, message, wParam, lParam);
}

bool MainGUI::WantCaptureKeyboard()
{
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureKeyboard;
}

bool MainGUI::WantCaptureMouse()
{
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureMouse;
}

void MainGUI::_onGUIScene(const int sceneIndex, bool& isContextMenuOpen)
{
	Scene* scene = m_client->scenes()[sceneIndex];
	if (NULL == scene)
		return;

	string64 sceneName;
	sceneName.format("Scene_%d", sceneIndex);
	if (ImGui::TreeNode(sceneName.c_str()))
	{
		for (int i = 0; i < scene->objectCount(); ++i)
			_onGUIObject(scene->object(i), isContextMenuOpen);

		ImGui::TreePop();
	}
}


void MainGUI::_onGUIObject(Object* const object, bool& isContextMenuOpen)
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


		ImVec2 buttonSize(200, 0);

	if (object->childCount() == 0)
	{
		//ui::Text(objectName.c_str());
		int node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
		ImGui::TreeNodeEx(objectName.c_str(), node_flags);
		if (!isContextMenuOpen && ImGui::BeginPopupContextItem())
		{
		   // your popup code
			if (ImGui::Button("context menu item 1", buttonSize))
			{
				printf("context menu pressed. object name = %s\n", objectName.c_str());
			}
			if (ImGui::Button("Close", buttonSize))
				ImGui::CloseCurrentPopup();
			isContextMenuOpen = true;
		   ImGui::EndPopup();
		}
		if (ImGui::IsItemClicked())
		{
			m_selectObject = object;
		}
	}
	else
	{
		bool nodeOpen = ImGui::TreeNodeEx(objectName.c_str(), 2240);
		if (!isContextMenuOpen && ImGui::BeginPopupContextItem())
		{
		   // your popup code
			if (ImGui::Button("context menu item 1", buttonSize))
			{
				printf("context menu pressed. object name = %s\n", objectName.c_str());
			}
			if (ImGui::Button("Close", buttonSize))
				ImGui::CloseCurrentPopup();
			isContextMenuOpen = true;
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
				_onGUIObject(object->child(i), isContextMenuOpen);
			}
			ImGui::TreePop();
		}
	}

}

void MainGUI::_onGUIProperty(Object* const object)
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

} // namespace cat



