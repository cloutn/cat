#include "./mainGUI.h"

#include "./client.h"
#include "./config.h"

#include "cat/object.h"
#include "cat/scene.h"
#include "cat/camera.h"

#include "scl/file.h"
#include "scl/vector.h"
#include "scl/matrix.h"
#include "scl/quaternion.h"
#include "scl/log.h"

#include "imgui_impl_win32.h"

#include "./imguiex.h"
#include "./ImGuizmo.h"


#ifdef SCL_WIN
#include <Windows.h>
#endif

#ifdef DEVICE_TYPE
#undef DEVICE_TYPE
#endif

#ifdef SCL_WIN
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

//#define HZ_CORE_IMGUI_COMPONENT_VAR(func, label, code) ImGui::TextUnformatted(label); ImGui::NextColumn(); ImGui::SetNextItemWidth(-1); if(func) { code } ImGui::NextColumn();

namespace cat {

using scl::file;
using scl::matrix;
using scl::vector3;
using scl::quaternion;

namespace ui = imguiex;
namespace imgui = ImGui;
namespace gizmo = ImGuizmo;

gizmo::OPERATION _operateTypeToGizmo(OPERATE_TYPE type)
{
	switch (type)
	{
	case OPERATE_TYPE_TRANSLATE : return gizmo::OPERATION::TRANSLATE;
	case OPERATE_TYPE_ROTATE	: return gizmo::OPERATION::ROTATE;
	case OPERATE_TYPE_SCALE		: return gizmo::OPERATION::SCALE;
	default						: assert(false); break;
	};

	return gizmo::OPERATION::TRANSLATE;
}

MainGUI::MainGUI() : m_client(NULL)//, m_selectObject(NULL) 
{

}

void MainGUI::init(Client* client)
{
	m_client = client;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 

	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Multi-Viewport / Platform Windows

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

	gizmo::Style& style = gizmo::GetStyle();
	float scale = 1.5;
	style.TranslationLineThickness		*= 0.5f;
	style.TranslationLineArrowSize		*= scale;
	style.RotationLineThickness			*= 0.5f;
	style.RotationOuterLineThickness	*= 0.5f;
	style.ScaleLineThickness			*= 0.5f;
	style.ScaleLineCircleSize			*= scale;
	style.CenterCircleSize				*= scale;
	style.HatchedAxisLineThickness		= 0;
	style.Colors[gizmo::DIRECTION_X]	= ImVec4(222/255.f,	56/255.f,	80/255.f,	1.000f); 
	style.Colors[gizmo::DIRECTION_Y]	= ImVec4(133/255.f,	210/255.f,	12/255.f,	1.000f); 
	style.Colors[gizmo::DIRECTION_Z]	= ImVec4(46/255.f,	134/255.f,	233/255.f,	1.000f); 
	style.ViewManipulateFont			= myshFont;
	style.ViewManipulateFontSize		= 18;
	gizmo::AllowAxisFlip(false);
}


void MainGUI::release()
{
	m_client->render().releaseIMGUI();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


void MainGUI::onGUI()
{
	if (NULL == m_client)
		return;

	_beginFrame();

	_processGizmo();

	_showMenu();

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	_showWindowScene();

	_showWindowProperty();

	_showWindowDebug();

	_showToolbar();

	_endFrame();
}

void MainGUI::Render()
{
	// Rendering
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
	if (!is_minimized)
	{
		m_client->render().drawIMGUI(draw_data);
	}
}

void MainGUI::onEvent(void* hWnd, uint32 message, uint32 wParam, uint32 lParam)
{
	ImGui_ImplWin32_WndProcHandler((HWND)hWnd, message, wParam, lParam);
}

bool MainGUI::wantCaptureKeyboard()
{
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureKeyboard;
}

bool MainGUI::wantCaptureMouse()
{
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureMouse;
}

void MainGUI::registerEvent(GUI_EVENT event, GUIEventFuncT func)
{
	m_events[event] = func;	
}


void MainGUI::_showWindowScene()
{
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
}

void MainGUI::_onGUIScene(const int sceneIndex, bool& isContextMenuOpen)
{
	Scene* scene = m_client->scenes()[sceneIndex];
	if (NULL == scene)
		return;

	string64 sceneName;
	sceneName.format("Scene_%d", sceneIndex);

	bool hasSelectedChild = (NULL != scene->objectByID(m_client->getSelectObjectID(), true));
	if (m_forceOpenSceneTree && hasSelectedChild)
	{
		ImGui::SetNextItemOpen(hasSelectedChild);
		m_forceOpenSceneTree = false;
	}

	if (ImGui::TreeNode(sceneName.c_str()))
	{
		for (int i = 0; i < scene->objectCount(); ++i)
			_onGUIObject(scene->object(i), isContextMenuOpen);

		ImGui::TreePop();
	}
}


void MainGUI::_onGUIObject(Object* const object, bool& isContextMenuOpen)
{
	string128 objectName = object->name().c_str();	
	if (objectName.empty())
		objectName = "[NoName]";

	ImVec2 buttonSize(200, 0);

	if (object->childCount() == 0)
	{
		//ui::Text(objectName.c_str());
		int node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
		if (m_client->getSelectObject() == object)
		{
			node_flags |= ImGuiTreeNodeFlags_Selected;
		}
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
			m_client->setSelectObject(object);
		}
	}
	else
	{
		int flag = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool hasSelectedChild = object->childByID(m_client->getSelectObjectID(), true);
		if (hasSelectedChild)
			flag |= ImGuiTreeNodeFlags_DefaultOpen;
		bool nodeOpen = ImGui::TreeNodeEx(objectName.c_str(), flag);
		//ImGuiTreeNodeFlags_Selected
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
			m_client->setSelectObject(object);
		}


		if (nodeOpen || hasSelectedChild)
		{
			for (int i = 0; i < object->childCount(); ++i)
			{
				_onGUIObject(object->child(i), isContextMenuOpen);
			}
			//ImGui::TreePop();
		}

		if (nodeOpen)
			ImGui::TreePop();
	}

}

void MainGUI::_beginFrame()
{
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void MainGUI::_endFrame()
{

}

void MainGUI::_fireEvent(GUI_EVENT event, GUIEvent& eventArg)
{
	m_events[event](eventArg);
}

void MainGUI::_processGizmo()
{
	Object* object = m_client->getSelectObject();
	if (NULL == object)
		return;

	gizmo::SetOrthographic	(false);	
	gizmo::SetRect			(0, 0, m_client->getScreenWidth(), m_client->getScreenHeight());

	Camera*					camera				= m_client->getCamera();
	const TRANSFORM_TYPE	transformType		= m_client->transformType();
	const OPERATE_TYPE		operateType			= m_client->operateType();
	gizmo::OPERATION		gizmoOperation		= _operateTypeToGizmo(operateType);

	// view handle manipulate
	scl::matrix				viewMatrixAfter		= camera->viewMatrix();
	const scl::matrix&		projectionMatrix	= camera->projectionMatrix();
	scl::matrix				matrix				= scl::matrix::identity();

	//gizmo::ViewManipulate(viewMatrixAfter.ptr(), projectionMatrix.ptr(), gizmoOperation, gizmo::WORLD, matrix.ptr(), 4.0f, ImVec2(100, 100), ImVec2(128, 128), 0x10101010);

	//gizmo::ViewManipulate(viewMatrixAfter.ptr(), projectionMatrix.ptr(), gizmoOperation, gizmo::WORLD, matrix.ptr(), 4.0f, ImVec2(0, 0), ImVec2(m_client->getScreenWidth(), m_client->getScreenHeight()), 0x10101010);
	//gizmo::ViewManipulateAxis(viewMatrixAfter.ptr(), projectionMatrix.ptr(), gizmoOperation, gizmo::WORLD, matrix.ptr(), 4.0f, 0.1f, ImVec2(0, 0), ImVec2(m_client->getScreenWidth(), m_client->getScreenHeight()), 0x10101010);

	gizmo::ViewManipulateAxis(viewMatrixAfter.ptr(), projectionMatrix.ptr(), gizmoOperation, gizmo::WORLD, matrix.ptr(), 4.0f, 0.2f, ImVec2(100, 100), ImVec2(128, 128), 0x10101010);

	if (!(viewMatrixAfter == camera->viewMatrix()))
	{
		camera->setViewByMatrix(viewMatrixAfter);
	}

	if (TRANSFORM_TYPE_LOCAL == transformType)
	{
		_operateLocal(object, camera, m_client->operateType());
	}
	else if (TRANSFORM_TYPE_GLOBAL == transformType)
	{
		_operateGlobal(object, camera, m_client->operateType());
	}
}

void MainGUI::_operateLocal(Object* object, Camera* camera, OPERATE_TYPE operateType)
{
	const scl::matrix&		viewMatrix			= camera->viewMatrix();
	const scl::matrix&		projectionMatrix	= camera->projectionMatrix();
	gizmo::OPERATION		gizmoOperation		= _operateTypeToGizmo(operateType);

	scl::matrix				matrix				= object->globalMatrix();
	scl::matrix				inverseParentMatrix = object->parentGlobalMatrixInverse();

	gizmo::Manipulate(viewMatrix.ptr(), projectionMatrix.ptr(), gizmoOperation, gizmo::LOCAL, matrix.ptr());
	if (!gizmo::IsUsing())
		return;

	scl::matrix				localMatrix		= matrix * inverseParentMatrix;
	vector3					translate			= { 0 };
	vector3					scale				= { 0 };
	quaternion				rotate				= { 0 };

	matrix::decompose(localMatrix, &translate, &scale, NULL, NULL, &rotate);

	//printf("before local transform : = %.3f, %.3f, %.3f\n", _localPrevPosition.x, _localPrevPosition.y, _localPrevPosition.z);
	//printf("after local transform : = %.3f, %.3f, %.3f\n----------\n", translate.x, translate.y, translate.z);
		
	object->setMove(translate);
	object->setScale(scale);
	object->setRotate(rotate);
}

void MainGUI::_operateGlobal(Object* object, Camera* camera, OPERATE_TYPE operateType)
{
	//printf("moving local");
	const scl::matrix&		viewMatrix			= camera->viewMatrix();
	const scl::matrix&		projectionMatrix	= camera->projectionMatrix();
	gizmo::OPERATION		gizmoOperation		= _operateTypeToGizmo(operateType);

	scl::matrix				matrix				= object->globalMatrix();
	scl::matrix				parentMatrix		= object->parentGlobalMatrix();
	scl::matrix				inverseParentMatrix = object->parentGlobalMatrixInverse();
	scl::vector3			globalPos			= scl::matrix::extract_move(matrix);
	scl::matrix				afterMatrix			= scl::matrix::move(globalPos.x, globalPos.y, globalPos.z);

	gizmo::Manipulate(viewMatrix.ptr(), projectionMatrix.ptr(), gizmoOperation, gizmo::WORLD, afterMatrix.ptr());
	if (!gizmo::IsUsing())
		return;

	if (OPERATE_TYPE_TRANSLATE == m_client->operateType())
	{
		_operateGlobalTranslate(object, afterMatrix, inverseParentMatrix);
	}
	else if (OPERATE_TYPE_ROTATE == m_client->operateType())
	{
		_operateGlobalRotate2(object, afterMatrix, parentMatrix, inverseParentMatrix);	
	}
}


void MainGUI::_operateGlobalTranslate(Object* object, scl::matrix afterMatrix, const scl::matrix& inverseParentMatrix)
{
	if (afterMatrix == matrix::identity())
		return;

	scl::matrix			localMatrix			= afterMatrix * inverseParentMatrix;
	vector3				localMove			= localMatrix.extract_move();
	object->setMove(localMove);
}

void MainGUI::_operateGlobalRotate(Object* object, scl::matrix afterMatrix, const scl::matrix& parentMatrix, const scl::matrix& inverseParentMatrix)
{
	afterMatrix.set_move(0, 0, 0);
	vector3				euler;
	matrix::decompose_rotation_xyz_radian(afterMatrix, euler);
	vector3				globalPivot;
	float				rotateRadian = 0;
	if (!scl::float_equal(euler.x, 0))
	{
		assert(scl::float_equal(euler.y, 0));
		assert(scl::float_equal(euler.z, 0));
		globalPivot.set(1, 0, 0);
		rotateRadian = euler.x;
	}
	else if (!scl::float_equal(euler.y, 0))
	{
		assert(scl::float_equal(euler.x, 0));
		assert(scl::float_equal(euler.z, 0));
		globalPivot.set(0, 1, 0);
		rotateRadian = euler.y;
	}
	else if (!scl::float_equal(euler.z, 0))
	{
		assert(scl::float_equal(euler.x, 0));
		assert(scl::float_equal(euler.y, 0));
		globalPivot.set(0, 0, 1);
		rotateRadian = euler.z;
	}
	vector3				parentPosition	= matrix::extract_move(parentMatrix);
	globalPivot += parentPosition;

	vector3				pivotInLocal	= globalPivot;
	pivotInLocal.mul_matrix(inverseParentMatrix);

	quaternion			localAddRotate;
	localAddRotate.from_pivot_radian(pivotInLocal, rotateRadian);

	matrix				localAddMatrix;
	localAddRotate.to_matrix(localAddMatrix);

	matrix				localMatrix = object->matrix() * localAddMatrix;

	vector3				translate			= { 0 };
	vector3				scale				= { 0 };
	quaternion			rotate				= { 0 };

	matrix::decompose(localMatrix, &translate, &scale, NULL, NULL, &rotate);

	object->setMove(translate);
	object->setScale(scale);
	object->setRotate(rotate);

	//printf("global rotate = %.3f\t%.3f\t%.3f\n", euler.x, euler.y, euler.z);
}

//void _print_matrix(const float* m)
//{
//	for (int i = 0; i < 4; ++i)
//	{
//		for (int j = 0; j < 4; ++j)
//			printf("%.4f\t", m[i * 4 + j]);
//		printf("\n");
//	}
//}

void MainGUI::_operateGlobalRotate2(Object* object, scl::matrix afterMatrix, const scl::matrix& parentMatrix, const scl::matrix& inverseParentMatrix)
{
	if (afterMatrix == matrix::identity())
		return;

	//_print_matrix(afterTransform.ptr());
	// 1. 计算 afterMatrix 中的旋转轴 pivot
	// 2. 将 pivot 变换到当前物体的本地空间中
	// 3. 在本地空间中计算绕 pivot 的旋转矩阵，
	// 4. 计算当前物体变换到原点的矩阵 -pos.x, -pos.y, -pos.z
	// 5. 用4中的矩阵程序3中的旋转矩阵
	// 6. 让 localMatrix 乘以这个矩阵，获得最终结果

	quaternion afterRotate;
	matrix::decompose(afterMatrix, NULL, NULL, NULL, NULL, &afterRotate);
	//printf("after Rotate xyzw = %f, %f, %f, %.9f\n", afterRotate.x, afterRotate.y, afterRotate.z, afterRotate.w);

	scl::vector3	globalPivot = { 0 };
	float			radian = 0;
	afterRotate.to_pivot_radian(globalPivot, radian);
	//printf("global pivot = %.3f, %.3f, %.3f, radian = %.9f\n", globalPivot.x, globalPivot.y, globalPivot.z, radian);

	vector3			parentTranslate = object->parentGlobalMatrix().extract_move();
	globalPivot += parentTranslate;		// counteract the translate in inverseParentMatrix

	vector3			localPivot = globalPivot * inverseParentMatrix;

	//printf("local pivot = %.3f, %.3f, %.3f\n", localPivot.x, localPivot.y, localPivot.z);

	quaternion		localAddRotate;
	localAddRotate.from_pivot_radian(localPivot, radian);

	matrix			localAddRotateMatrix;
	localAddRotate.to_matrix(localAddRotateMatrix);

	vector3			pos				= object->position();
	matrix			localAddMatrix = matrix::move(-pos.x, -pos.y, -pos.z);
	localAddMatrix.mul(localAddRotateMatrix);
	localAddMatrix.mul(matrix::move(pos.x, pos.y, pos.z));

	matrix			localMatrix = object->matrix() * localAddMatrix;

	vector3			translate			= { 0 };
	vector3			scale				= { 0 };
	quaternion		rotate				= { 0 };

	matrix::decompose(localMatrix, &translate, &scale, NULL, NULL, &rotate);

	object->setMove(translate);
	object->setScale(scale);
	object->setRotate(rotate);
}

void MainGUI::_showWindowProperty()
{
	Object* object = m_client->getSelectObject();
	if (NULL == object)
		return;

	ImGui::Begin("Property");   

	string512 name = object->name().c_str();
	//ImGui::InputText("Name", name.c_str(), name.capacity());
	ui::inputText("Name", name.c_str(), name.capacity());
	object->setName(name.c_str());

	// TODO 现在还无法直接修改 matrix，可以直接使用 decompose 修改
	scl::matrix mat = object->matrix();
	ui::inputMatrix4("Transform", mat);

	// position
	scl::vector3 pos = object->position();
	ui::inputFloat3("Position", pos);
	object->setPosition(pos);

	// scale
	scl::vector3 scale = object->scale();
	ui::inputFloat3("Scale", scale);
	object->setScale(scale);

	// rotate
	scl::vector3 rotate = object->rotateAngle();
	ui::inputFloat3("Rotate", rotate);
	object->setRotateAngle(rotate);

	// enable skin
	bool enableSkin = object->isEnableSkin();
	ui::checkbox("Enable Skin", enableSkin);
	object->setEnableSkin(enableSkin);

	// gltf index
	string16 gltfIndex;
	gltfIndex.from_int(object->gltfIndex());
	ui::labelText("Gltf Index", gltfIndex.c_str());

	bool enableAnimation = object->isEnableAnimation();
	ui::checkbox("Enable Animation", enableAnimation);
	object->setEnableAnimation(enableAnimation);

	ImGui::End();
}

void MainGUI::_showWindowDebug()
{
	game::Config&	config = m_client->config();

	ImGui::Begin("Debug main");                          

	// show fps
	string256 fpsStr;
	float fps = m_client->fps();
	fpsStr.format("fps = %.2f", fps);
	ImGui::Text(fpsStr.c_str());               

	// show mouse position
	string128 mousePositionStr;
	mousePositionStr.format("mouse {%d, %d}", m_client->mousePosition().x, m_client->mousePosition().y);
	ImGui::Text(mousePositionStr.c_str());              

	// select object
	string512 strSelect = "select : [";
	Object* selectObject = m_client->getSelectObject();
	if (NULL != selectObject)
		strSelect += selectObject->name().c_str();
	else
		strSelect += "None";
	strSelect += "]";
	ImGui::Text(strSelect.c_str());

	if (imgui::Button("show demo window", {imgui::CalcItemWidth(), 0}))
		config.showDemoWindow = !config.showDemoWindow;
	if (config.showDemoWindow)
		ImGui::ShowDemoWindow(&config.showDemoWindow);

	if (ImGui::Button("Do pick pass", {imgui::CalcItemWidth(), 0}))
		_fireEvent(GUI_EVENT_PICK_PASS_CLICK, GUIEvent());

	if (imgui::Button("show device window", {imgui::CalcItemWidth(), 0}))
		config.showDeviceInfoWindow = !config.showDeviceInfoWindow;
	if (config.showDeviceInfoWindow)
		_showWindowDeviceInfo();

	if (imgui::Button("show config window", {imgui::CalcItemWidth(), 0}))
		config.showConfigWindow = !config.showConfigWindow;
	if (config.showConfigWindow)
		_showWindowConfig();

	ImGui::End();
}

const char* const _deviceTypeToString(DeviceInfo::DEVICE_TYPE t)
{
	switch (t)
	{
	case DeviceInfo::DEVICE_TYPE_OTHER			: return "other unknow";
	case DeviceInfo::DEVICE_TYPE_INTEGRATED_GPU	: return "intergrated GPU";
	case DeviceInfo::DEVICE_TYPE_DISCRETE_GPU	: return "discrete GPU";
	case DeviceInfo::DEVICE_TYPE_VIRTUAL_GPU	: return "virtual GPU";
	default: return "";
	}
	return "";
}

void MainGUI::_showMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            //ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


void MainGUI::_showToolbar()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowBgAlpha(0.35f); 
    if (ImGui::Begin("Example: Simple overlay", NULL, window_flags))
    {
		bool isLocalTransform = m_client->transformType() == TRANSFORM_TYPE_LOCAL;
		ImGui::Checkbox("local", &isLocalTransform);
		if (!isLocalTransform)
		{
			if (m_client->operateType() == OPERATE_TYPE_SCALE) // scale can only in local transform
				isLocalTransform = true;
		}
		m_client->setTransformType(isLocalTransform ? TRANSFORM_TYPE_LOCAL : TRANSFORM_TYPE_GLOBAL);

		ImGui::SameLine();
		bool test2 = true;
		ImGui::Checkbox("test2", &test2);
    }
    ImGui::End();
}

void MainGUI::_showWindowConfig()
{
	game::Config& config = m_client->config();

	ImGui::Begin("Config", &m_client->config().showConfigWindow);

	ui::checkbox		("showDemoWindow",			config.showDemoWindow);
	ui::checkbox		("showDeviceInfoWindow",	config.showDeviceInfoWindow);
	ui::checkbox		("showConfigWindow",		config.showConfigWindow);
	ui::inputInt2		("screenSize",				config.screenSize);
	ui::inputInt2		("screenPos",				config.screenPos);
	ui::inputColorInt	("clearColor",				config.clearColor);

	ImGui::End();
}


void MainGUI::_showWindowDeviceInfo()
{
	ImGui::Begin("Device info", &m_client->config().showDeviceInfoWindow);

	// info
	const DeviceInfo& info = m_client->getDeviceInfo();
	ui::labelText("deviceType", _deviceTypeToString(info.deviceType));
	ui::labelText("deviceName", info.deviceName);
	ui::labelText("Vulkan API version", "%u.%u.%u", info.apiVersionMajor, info.apiVersionMinor, info.apiVersionPatch);

	// limits
	ui::labelText("limits.maxUniformBufferRange",				"%u", info.limits.maxUniformBufferRange);
	ui::labelText("limits.maxStorageBufferRange",				"%u", info.limits.maxStorageBufferRange);
	ui::labelText("limits.minUniformBufferOffsetAlignment",		"%llu", info.limits.minUniformBufferOffsetAlignment);
	ui::labelText("limits.maxPushConstantsSize",				"%u", info.limits.maxPushConstantsSize);
	ui::labelText("limits.maxBoundDescriptorSets",				"%u", info.limits.maxBoundDescriptorSets);

	// others info and limits
	if (ImGui::CollapsingHeader("Other info"))
    {
		ui::labelText("driverVersion", "%u", info.driverVersion);
		ui::labelText("vendorID", "%u", info.vendorID);
		ui::labelText("deviceID", "%u", info.deviceID);
		string128 guid;
		for (int i = 0; i < countof(info.pipelineCacheUUID); ++i)
			guid.format_append("%x:", info.pipelineCacheUUID[i]);
		ui::labelText("pipelineCacheUUID", guid.c_str());

		// limits
		ui::labelText("limits.maxImageDimension1D",									"%u", info.limits.maxImageDimension1D);
		ui::labelText("limits.maxImageDimension2D",									"%u", info.limits.maxImageDimension2D);
		ui::labelText("limits.maxImageDimension3D",									"%u", info.limits.maxImageDimension3D);
		ui::labelText("limits.maxImageDimensionCube",								"%u", info.limits.maxImageDimensionCube);
		ui::labelText("limits.maxImageArrayLayers",									"%u", info.limits.maxImageArrayLayers);
		ui::labelText("limits.maxTexelBufferElements",								"%u", info.limits.maxTexelBufferElements);
		ui::labelText("limits.maxMemoryAllocationCount",							"%u", info.limits.maxMemoryAllocationCount);
		ui::labelText("limits.maxSamplerAllocationCount",							"%u", info.limits.maxSamplerAllocationCount);
		ui::labelText("limits.bufferImageGranularity",								"%llu", info.limits.bufferImageGranularity);
		ui::labelText("limits.sparseAddressSpaceSize",								"%llu", info.limits.sparseAddressSpaceSize);
		ui::labelText("limits.maxPerStageDescriptorSamplers",						"%u", info.limits.maxPerStageDescriptorSamplers);
		ui::labelText("limits.maxPerStageDescriptorUniformBuffers",					"%u", info.limits.maxPerStageDescriptorUniformBuffers);
		ui::labelText("limits.maxPerStageDescriptorStorageBuffers",					"%u", info.limits.maxPerStageDescriptorStorageBuffers);
		ui::labelText("limits.maxPerStageDescriptorSampledImages",					"%u", info.limits.maxPerStageDescriptorSampledImages);
		ui::labelText("limits.maxPerStageDescriptorStorageImages",					"%u", info.limits.maxPerStageDescriptorStorageImages);
		ui::labelText("limits.maxPerStageDescriptorInputAttachments",				"%u", info.limits.maxPerStageDescriptorInputAttachments);
		ui::labelText("limits.maxPerStageResources",								"%u", info.limits.maxPerStageResources);
		ui::labelText("limits.maxDescriptorSetSamplers",							"%u", info.limits.maxDescriptorSetSamplers);
		ui::labelText("limits.maxDescriptorSetUniformBuffers",						"%u", info.limits.maxDescriptorSetUniformBuffers);
		ui::labelText("limits.maxDescriptorSetUniformBuffersDynamic",				"%u", info.limits.maxDescriptorSetUniformBuffersDynamic);
		ui::labelText("limits.maxDescriptorSetStorageBuffers",						"%u", info.limits.maxDescriptorSetStorageBuffers);
		ui::labelText("limits.maxDescriptorSetStorageBuffersDynamic",				"%u", info.limits.maxDescriptorSetStorageBuffersDynamic);
		ui::labelText("limits.maxDescriptorSetSampledImages",						"%u", info.limits.maxDescriptorSetSampledImages);
		ui::labelText("limits.maxDescriptorSetStorageImages",						"%u", info.limits.maxDescriptorSetStorageImages);
		ui::labelText("limits.maxDescriptorSetInputAttachments",					"%u", info.limits.maxDescriptorSetInputAttachments);
		ui::labelText("limits.maxVertexInputAttributes",							"%u", info.limits.maxVertexInputAttributes);
		ui::labelText("limits.maxVertexInputBindings",								"%u", info.limits.maxVertexInputBindings);
		ui::labelText("limits.maxVertexInputAttributeOffset",						"%u", info.limits.maxVertexInputAttributeOffset);
		ui::labelText("limits.maxVertexInputBindingStride",							"%u", info.limits.maxVertexInputBindingStride);
		ui::labelText("limits.maxVertexOutputComponents",							"%u", info.limits.maxVertexOutputComponents);
		ui::labelText("limits.maxTessellationGenerationLevel",						"%u", info.limits.maxTessellationGenerationLevel);
		ui::labelText("limits.maxTessellationPatchSize",							"%u", info.limits.maxTessellationPatchSize);
		ui::labelText("limits.maxTessellationControlPerVertexInputComponents",		"%u", info.limits.maxTessellationControlPerVertexInputComponents);
		ui::labelText("limits.maxTessellationControlPerVertexOutputComponents",		"%u", info.limits.maxTessellationControlPerVertexOutputComponents);
		ui::labelText("limits.maxTessellationControlPerPatchOutputComponents",		"%u", info.limits.maxTessellationControlPerPatchOutputComponents);
		ui::labelText("limits.maxTessellationControlTotalOutputComponents",			"%u", info.limits.maxTessellationControlTotalOutputComponents);
		ui::labelText("limits.maxTessellationEvaluationInputComponents",			"%u", info.limits.maxTessellationEvaluationInputComponents);
		ui::labelText("limits.maxTessellationEvaluationOutputComponents",			"%u", info.limits.maxTessellationEvaluationOutputComponents);
		ui::labelText("limits.maxGeometryShaderInvocations",						"%u", info.limits.maxGeometryShaderInvocations);
		ui::labelText("limits.maxGeometryInputComponents",							"%u", info.limits.maxGeometryInputComponents);
		ui::labelText("limits.maxGeometryOutputComponents",							"%u", info.limits.maxGeometryOutputComponents);
		ui::labelText("limits.maxGeometryOutputVertices",							"%u", info.limits.maxGeometryOutputVertices);
		ui::labelText("limits.maxGeometryTotalOutputComponents",					"%u", info.limits.maxGeometryTotalOutputComponents);
		ui::labelText("limits.maxFragmentInputComponents",							"%u", info.limits.maxFragmentInputComponents);
		ui::labelText("limits.maxFragmentOutputAttachments",						"%u", info.limits.maxFragmentOutputAttachments);
		ui::labelText("limits.maxFragmentDualSrcAttachments",						"%u", info.limits.maxFragmentDualSrcAttachments);
		ui::labelText("limits.maxFragmentCombinedOutputResources",					"%u", info.limits.maxFragmentCombinedOutputResources);
		ui::labelText("limits.maxComputeSharedMemorySize",							"%u", info.limits.maxComputeSharedMemorySize);
		ui::labelText("limits.maxComputeWorkGroupCount",							"%u, %u, %u", info.limits.maxComputeWorkGroupCount[0], info.limits.maxComputeWorkGroupCount[1], info.limits.maxComputeWorkGroupCount[2]);
		ui::labelText("limits.maxComputeWorkGroupInvocations",						"%u", info.limits.maxComputeWorkGroupInvocations);
		ui::labelText("limits.maxComputeWorkGroupSize",								"%u, %u, %u", info.limits.maxComputeWorkGroupSize[0], info.limits.maxComputeWorkGroupSize[1], info.limits.maxComputeWorkGroupSize[2]);
		ui::labelText("limits.subPixelPrecisionBits",								"%u", info.limits.subPixelPrecisionBits);
		ui::labelText("limits.subTexelPrecisionBits",								"%u", info.limits.subTexelPrecisionBits);
		ui::labelText("limits.mipmapPrecisionBits",									"%u", info.limits.mipmapPrecisionBits);
		ui::labelText("limits.maxDrawIndexedIndexValue",							"%u", info.limits.maxDrawIndexedIndexValue);
		ui::labelText("limits.maxDrawIndirectCount",								"%u", info.limits.maxDrawIndirectCount);
		ui::labelText("limits.maxSamplerLodBias",									"%f", info.limits.maxSamplerLodBias);
		ui::labelText("limits.maxSamplerAnisotropy",								"%f", info.limits.maxSamplerAnisotropy);
		ui::labelText("limits.maxViewports",										"%u", info.limits.maxViewports);
		ui::labelText("limits.maxViewportDimensions",								"%u, %u", info.limits.maxViewportDimensions[0], info.limits.maxViewportDimensions[1]);
		ui::labelText("limits.viewportBoundsRange",									"%f, %f", info.limits.viewportBoundsRange[0], info.limits.maxViewportDimensions[1]);
		ui::labelText("limits.viewportSubPixelBits",								"%u", info.limits.viewportSubPixelBits);
		ui::labelText("limits.minMemoryMapAlignment",								"%llu", info.limits.minMemoryMapAlignment);
		ui::labelText("limits.minTexelBufferOffsetAlignment",						"%llu", info.limits.minTexelBufferOffsetAlignment);
		ui::labelText("limits.minStorageBufferOffsetAlignment",						"%llu", info.limits.minStorageBufferOffsetAlignment);
		ui::labelText("limits.minTexelOffset",										"%d", info.limits.minTexelOffset);
		ui::labelText("limits.maxTexelOffset",										"%u", info.limits.maxTexelOffset);
		ui::labelText("limits.minTexelGatherOffset",								"%d", info.limits.minTexelGatherOffset);
		ui::labelText("limits.maxTexelGatherOffset",								"%u", info.limits.maxTexelGatherOffset);
		ui::labelText("limits.minInterpolationOffset",								"%f", info.limits.minInterpolationOffset);
		ui::labelText("limits.maxInterpolationOffset",								"%f", info.limits.maxInterpolationOffset);
		ui::labelText("limits.subPixelInterpolationOffsetBits",						"%u", info.limits.subPixelInterpolationOffsetBits);
		ui::labelText("limits.maxFramebufferWidth",									"%u", info.limits.maxFramebufferWidth);
		ui::labelText("limits.maxFramebufferHeight",								"%u", info.limits.maxFramebufferHeight);
		ui::labelText("limits.maxFramebufferLayers",								"%u", info.limits.maxFramebufferLayers);
		ui::labelText("limits.framebufferColorSampleCounts",						"%u", info.limits.framebufferColorSampleCounts);
		ui::labelText("limits.framebufferDepthSampleCounts",						"%u", info.limits.framebufferDepthSampleCounts);
		ui::labelText("limits.framebufferStencilSampleCounts",						"%u", info.limits.framebufferStencilSampleCounts);
		ui::labelText("limits.framebufferNoAttachmentsSampleCounts",				"%u", info.limits.framebufferNoAttachmentsSampleCounts);
		ui::labelText("limits.maxColorAttachments",									"%u", info.limits.maxColorAttachments);
		ui::labelText("limits.sampledImageColorSampleCounts",						"%u", info.limits.sampledImageColorSampleCounts);
		ui::labelText("limits.sampledImageIntegerSampleCounts",						"%u", info.limits.sampledImageIntegerSampleCounts);
		ui::labelText("limits.sampledImageDepthSampleCounts",						"%u", info.limits.sampledImageDepthSampleCounts);
		ui::labelText("limits.sampledImageStencilSampleCounts",						"%u", info.limits.sampledImageStencilSampleCounts);
		ui::labelText("limits.storageImageSampleCounts",							"%u", info.limits.storageImageSampleCounts);
		ui::labelText("limits.maxSampleMaskWords",									"%u", info.limits.maxSampleMaskWords);
		ui::labelText("limits.timestampComputeAndGraphics",							"%u", info.limits.timestampComputeAndGraphics);
		ui::labelText("limits.timestampPeriod",										"%f", info.limits.timestampPeriod);
		ui::labelText("limits.maxClipDistances",									"%u", info.limits.maxClipDistances);
		ui::labelText("limits.maxCullDistances",									"%u", info.limits.maxCullDistances);
		ui::labelText("limits.maxCombinedClipAndCullDistances",						"%u", info.limits.maxCombinedClipAndCullDistances);
		ui::labelText("limits.discreteQueuePriorities",								"%u", info.limits.discreteQueuePriorities);
		ui::labelText("limits.pointSizeRange",										"%f, %f", info.limits.pointSizeRange[0], info.limits.pointSizeRange[1]);
		ui::labelText("limits.lineWidthRange",										"%f, %f", info.limits.lineWidthRange[0], info.limits.lineWidthRange[1]);
		ui::labelText("limits.pointSizeGranularity",								"%f", info.limits.pointSizeGranularity);
		ui::labelText("limits.lineWidthGranularity",								"%f", info.limits.lineWidthGranularity);
		ui::labelText("limits.strictLines",											"%u", info.limits.strictLines);
		ui::labelText("limits.standardSampleLocations",								"%u", info.limits.standardSampleLocations);
		ui::labelText("limits.optimalBufferCopyOffsetAlignment",					"%llu", info.limits.optimalBufferCopyOffsetAlignment);
		ui::labelText("limits.optimalBufferCopyRowPitchAlignment",					"%llu", info.limits.optimalBufferCopyRowPitchAlignment);
		ui::labelText("limits.nonCoherentAtomSize",									"%llu", info.limits.nonCoherentAtomSize);
	}


	ImGui::End();
}


} // namespace cat




