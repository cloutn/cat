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

#ifdef DEVICE_TYPE
#undef DEVICE_TYPE
#endif

#ifdef SCL_WIN
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

//#define HZ_CORE_IMGUI_COMPONENT_VAR(func, label, code) ImGui::TextUnformatted(label); ImGui::NextColumn(); ImGui::SetNextItemWidth(-1); if(func) { code } ImGui::NextColumn();

namespace cat {

using scl::file;

void myLabelText(const char* const name, const char* valueFormat, ...) 
{ 
	va_list arg;
	va_start(arg, valueFormat);

	float x = ImGui::GetCursorPosX();
	ImGui::TextUnformatted(name);
	ImGui::SameLine();
	ImGui::SetCursorPosX(x + ImGui::CalcItemWidth() + ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::TextV(valueFormat, arg);

	va_end(arg);
}

void myInputDouble(const char* const label, double& v)
{
	float x = ImGui::GetCursorPosX();
	ImGui::Text(label); 
	ImGui::SameLine(); 
	ImGui::SetCursorPosX(x + ImGui::CalcItemWidth() + ImGui::GetStyle().ItemInnerSpacing.x);
	string512 labelID = "##";
	labelID += label;
	ImGui::InputDouble(labelID.c_str(), &v);
}

void myCheckbox(const char* const label, bool& v)
{
	float x = ImGui::GetCursorPosX();
	ImGui::Text(label); 
	ImGui::SameLine(); 
	ImGui::SetCursorPosX(x + ImGui::CalcItemWidth() + ImGui::GetStyle().ItemInnerSpacing.x);
	string512 labelID = "##";
	labelID += label;
	ImGui::Checkbox(labelID.c_str(), &v);

	//int total_w = ImGui::GetContentRegionAvail().x; 
	//ImGui::Text("lalala"); 
	//ImGui::SameLine(total_w); 
	//ImGui::SetNextItemWidth(total_w); 
	//ImGui::Checkbox("##", &v);
}

MainGUI::MainGUI() : m_client(NULL), m_selectObject(NULL) 
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

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	_windowScene();

	_windowProperty(m_selectObject);

	_windowDebug();

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


void MainGUI::_windowScene()
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

void MainGUI::_beginFrame()
{
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

void MainGUI::_endFrame()
{

}

void MainGUI::_fireEvent(GUI_EVENT event, GUIEvent& eventArg)
{
	m_events[event](eventArg);
}

void MainGUI::_windowProperty(Object* const object)
{
	if (NULL == object)
		return;

	ImGui::Begin("Property");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

	string128 name = object->name().c_str();
	ImGui::InputText("Name", name.c_str(), name.capacity());
	object->setName(name.c_str());

	ImGui::End();
}

void MainGUI::_windowDebug()
{
	game::Config& config = m_client->config();

	if (config.showDemoWindow)
		ImGui::ShowDemoWindow(&config.showDemoWindow);

	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin("Debug main");                          // Create a window called "Hello, world!" and append into it.

	string256 fpsStr;
	float fps = m_client->fps();
	fpsStr.format("fps = %.2f", fps);

	ImGui::Text(fpsStr.c_str());               // Display some text (you can use a format strings too)

	string128 mousePositionStr;
	mousePositionStr.format("mouse {%d, %d}", m_client->mousePosition().x, m_client->mousePosition().y);
	ImGui::Text(mousePositionStr.c_str());               // Display some text (you can use a format strings too)

	ImGui::Checkbox("Demo window", &config.showDemoWindow);      // Edit bools storing our window open/close state

	bool bv = false;
	myCheckbox("check 1", bv);
	double dv = 0;
	myInputDouble("double 2", dv);

	if (NULL != m_selectObject)
		ImGui::Text(m_selectObject->name().c_str());               // Display some text (you can use a format strings too)

	if (ImGui::Button("Do pick pass"))
	{
		//printf("context menu pressed.");
		//if (NULL != m_debugButton1ClickFunc)
		//m_debugButton1ClickFunc();
		_fireEvent(GUI_EVENT_DEBUG_BUTTON_CLICK, GUIEvent());
	}

	//if (ImGui::Button("Show device info"))
	//{
	//	_windowDeviceInfo();
	//}
	ImGui::Checkbox("Show device window", &config.showDeviceInfoWindow);      // Edit bools storing our window open/close state
	if (config.showDeviceInfoWindow)
		_windowDeviceInfo();

	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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



void MainGUI::_windowDeviceInfo()
{
	ImGui::Begin("Device info");

	// info
	const DeviceInfo& info = m_client->getDeviceInfo();
	myLabelText("deviceType", _deviceTypeToString(info.deviceType));
	myLabelText("deviceName", info.deviceName);
	myLabelText("Vulkan API version", "%u.%u.%u", info.apiVersionMajor, info.apiVersionMinor, info.apiVersionPatch);

	// limits
	myLabelText("limits.maxUniformBufferRange",								"%u", info.limits.maxUniformBufferRange);
	myLabelText("limits.maxStorageBufferRange",								"%u", info.limits.maxStorageBufferRange);
	myLabelText("limits.minUniformBufferOffsetAlignment",						"%llu", info.limits.minUniformBufferOffsetAlignment);
	myLabelText("limits.maxPushConstantsSize",									"%u", info.limits.maxPushConstantsSize);
	myLabelText("limits.maxBoundDescriptorSets",								"%u", info.limits.maxBoundDescriptorSets);



	// others info and limits
	if (ImGui::CollapsingHeader("Other info"))
    {
		myLabelText("driverVersion", "%u", info.driverVersion);
		myLabelText("vendorID", "%u", info.vendorID);
		myLabelText("deviceID", "%u", info.deviceID);
		string128 guid;
		for (int i = 0; i < countof(info.pipelineCacheUUID); ++i)
			guid.format_append("%x:", info.pipelineCacheUUID[i]);
		myLabelText("pipelineCacheUUID", guid.c_str());

		// limits
		myLabelText("limits.maxImageDimension1D",									"%u", info.limits.maxImageDimension1D);
		myLabelText("limits.maxImageDimension2D",									"%u", info.limits.maxImageDimension2D);
		myLabelText("limits.maxImageDimension3D",									"%u", info.limits.maxImageDimension3D);
		myLabelText("limits.maxImageDimensionCube",								"%u", info.limits.maxImageDimensionCube);
		myLabelText("limits.maxImageArrayLayers",									"%u", info.limits.maxImageArrayLayers);
		myLabelText("limits.maxTexelBufferElements",								"%u", info.limits.maxTexelBufferElements);
		//myLabelText("limits.maxUniformBufferRange",								"%u", info.limits.maxUniformBufferRange);
		//myLabelText("limits.maxStorageBufferRange",								"%u", info.limits.maxStorageBufferRange);
		//myLabelText("limits.maxPushConstantsSize",									"%u", info.limits.maxPushConstantsSize);
		myLabelText("limits.maxMemoryAllocationCount",								"%u", info.limits.maxMemoryAllocationCount);
		myLabelText("limits.maxSamplerAllocationCount",							"%u", info.limits.maxSamplerAllocationCount);
		myLabelText("limits.bufferImageGranularity",								"%llu", info.limits.bufferImageGranularity);
		myLabelText("limits.sparseAddressSpaceSize",								"%llu", info.limits.sparseAddressSpaceSize);
		//myLabelText("limits.maxBoundDescriptorSets",								"%u", info.limits.maxBoundDescriptorSets);
		myLabelText("limits.maxPerStageDescriptorSamplers",						"%u", info.limits.maxPerStageDescriptorSamplers);
		myLabelText("limits.maxPerStageDescriptorUniformBuffers",					"%u", info.limits.maxPerStageDescriptorUniformBuffers);
		myLabelText("limits.maxPerStageDescriptorStorageBuffers",					"%u", info.limits.maxPerStageDescriptorStorageBuffers);
		myLabelText("limits.maxPerStageDescriptorSampledImages",					"%u", info.limits.maxPerStageDescriptorSampledImages);
		myLabelText("limits.maxPerStageDescriptorStorageImages",					"%u", info.limits.maxPerStageDescriptorStorageImages);
		myLabelText("limits.maxPerStageDescriptorInputAttachments",				"%u", info.limits.maxPerStageDescriptorInputAttachments);
		myLabelText("limits.maxPerStageResources",									"%u", info.limits.maxPerStageResources);
		myLabelText("limits.maxDescriptorSetSamplers",								"%u", info.limits.maxDescriptorSetSamplers);
		myLabelText("limits.maxDescriptorSetUniformBuffers",						"%u", info.limits.maxDescriptorSetUniformBuffers);
		myLabelText("limits.maxDescriptorSetUniformBuffersDynamic",				"%u", info.limits.maxDescriptorSetUniformBuffersDynamic);
		myLabelText("limits.maxDescriptorSetStorageBuffers",						"%u", info.limits.maxDescriptorSetStorageBuffers);
		myLabelText("limits.maxDescriptorSetStorageBuffersDynamic",				"%u", info.limits.maxDescriptorSetStorageBuffersDynamic);
		myLabelText("limits.maxDescriptorSetSampledImages",						"%u", info.limits.maxDescriptorSetSampledImages);
	
		myLabelText("limits.maxDescriptorSetStorageImages",						"%u", info.limits.maxDescriptorSetStorageImages);
		myLabelText("limits.maxDescriptorSetInputAttachments",						"%u", info.limits.maxDescriptorSetInputAttachments);
		myLabelText("limits.maxVertexInputAttributes",								"%u", info.limits.maxVertexInputAttributes);
		myLabelText("limits.maxVertexInputBindings",								"%u", info.limits.maxVertexInputBindings);
		myLabelText("limits.maxVertexInputAttributeOffset",						"%u", info.limits.maxVertexInputAttributeOffset);
		myLabelText("limits.maxVertexInputBindingStride",							"%u", info.limits.maxVertexInputBindingStride);
		myLabelText("limits.maxVertexOutputComponents",							"%u", info.limits.maxVertexOutputComponents);
		myLabelText("limits.maxTessellationGenerationLevel",						"%u", info.limits.maxTessellationGenerationLevel);
		myLabelText("limits.maxTessellationPatchSize",								"%u", info.limits.maxTessellationPatchSize);
		myLabelText("limits.maxTessellationControlPerVertexInputComponents",		"%u", info.limits.maxTessellationControlPerVertexInputComponents);
		myLabelText("limits.maxTessellationControlPerVertexOutputComponents",		"%u", info.limits.maxTessellationControlPerVertexOutputComponents);
		myLabelText("limits.maxTessellationControlPerPatchOutputComponents",		"%u", info.limits.maxTessellationControlPerPatchOutputComponents);
		myLabelText("limits.maxTessellationControlTotalOutputComponents",			"%u", info.limits.maxTessellationControlTotalOutputComponents);
		myLabelText("limits.maxTessellationEvaluationInputComponents",				"%u", info.limits.maxTessellationEvaluationInputComponents);
		myLabelText("limits.maxTessellationEvaluationOutputComponents",			"%u", info.limits.maxTessellationEvaluationOutputComponents);
		myLabelText("limits.maxGeometryShaderInvocations",							"%u", info.limits.maxGeometryShaderInvocations);
		myLabelText("limits.maxGeometryInputComponents",							"%u", info.limits.maxGeometryInputComponents);
		myLabelText("limits.maxGeometryOutputComponents",							"%u", info.limits.maxGeometryOutputComponents);
		myLabelText("limits.maxGeometryOutputVertices",							"%u", info.limits.maxGeometryOutputVertices);
		myLabelText("limits.maxGeometryTotalOutputComponents",						"%u", info.limits.maxGeometryTotalOutputComponents);
		myLabelText("limits.maxFragmentInputComponents",							"%u", info.limits.maxFragmentInputComponents);
		myLabelText("limits.maxFragmentOutputAttachments",							"%u", info.limits.maxFragmentOutputAttachments);
		myLabelText("limits.maxFragmentDualSrcAttachments",						"%u", info.limits.maxFragmentDualSrcAttachments);
		myLabelText("limits.maxFragmentCombinedOutputResources",					"%u", info.limits.maxFragmentCombinedOutputResources);
		myLabelText("limits.maxComputeSharedMemorySize",							"%u", info.limits.maxComputeSharedMemorySize);
		myLabelText("limits.maxComputeWorkGroupCount",								"%u, %u, %u", info.limits.maxComputeWorkGroupCount[0], info.limits.maxComputeWorkGroupCount[1], info.limits.maxComputeWorkGroupCount[2]);
		myLabelText("limits.maxComputeWorkGroupInvocations",						"%u", info.limits.maxComputeWorkGroupInvocations);
		myLabelText("limits.maxComputeWorkGroupSize",								"%u, %u, %u", info.limits.maxComputeWorkGroupSize[0], info.limits.maxComputeWorkGroupSize[1], info.limits.maxComputeWorkGroupSize[2]);
		myLabelText("limits.subPixelPrecisionBits",								"%u", info.limits.subPixelPrecisionBits);
		myLabelText("limits.subTexelPrecisionBits",								"%u", info.limits.subTexelPrecisionBits);
		myLabelText("limits.mipmapPrecisionBits",									"%u", info.limits.mipmapPrecisionBits);
		myLabelText("limits.maxDrawIndexedIndexValue",								"%u", info.limits.maxDrawIndexedIndexValue);
		myLabelText("limits.maxDrawIndirectCount",									"%u", info.limits.maxDrawIndirectCount);
		myLabelText("limits.maxSamplerLodBias",									"%f", info.limits.maxSamplerLodBias);
		myLabelText("limits.maxSamplerAnisotropy",									"%f", info.limits.maxSamplerAnisotropy);
		myLabelText("limits.maxViewports",											"%u", info.limits.maxViewports);
#include "scl/array.h"
#include "scl/function.h"
		myLabelText("limits.maxViewportDimensions",								"%u, %u", info.limits.maxViewportDimensions[0], info.limits.maxViewportDimensions[1]);
		myLabelText("limits.viewportBoundsRange",									"%f, %f", info.limits.viewportBoundsRange[0], info.limits.maxViewportDimensions[1]);
		myLabelText("limits.viewportSubPixelBits",									"%u", info.limits.viewportSubPixelBits);
		myLabelText("limits.minMemoryMapAlignment",								"%llu", info.limits.minMemoryMapAlignment);
		myLabelText("limits.minTexelBufferOffsetAlignment",						"%llu", info.limits.minTexelBufferOffsetAlignment);
		//myLabelText("limits.minUniformBufferOffsetAlignment",						"%llu", info.limits.minUniformBufferOffsetAlignment);
		myLabelText("limits.minStorageBufferOffsetAlignment",						"%llu", info.limits.minStorageBufferOffsetAlignment);
		myLabelText("limits.minTexelOffset",										"%d", info.limits.minTexelOffset);
		myLabelText("limits.maxTexelOffset",										"%u", info.limits.maxTexelOffset);
		myLabelText("limits.minTexelGatherOffset",									"%d", info.limits.minTexelGatherOffset);
		myLabelText("limits.maxTexelGatherOffset",									"%u", info.limits.maxTexelGatherOffset);
		myLabelText("limits.minInterpolationOffset",								"%f", info.limits.minInterpolationOffset);
		myLabelText("limits.maxInterpolationOffset",								"%f", info.limits.maxInterpolationOffset);
		myLabelText("limits.subPixelInterpolationOffsetBits",						"%u", info.limits.subPixelInterpolationOffsetBits);
		myLabelText("limits.maxFramebufferWidth",									"%u", info.limits.maxFramebufferWidth);
		myLabelText("limits.maxFramebufferHeight",									"%u", info.limits.maxFramebufferHeight);
		myLabelText("limits.maxFramebufferLayers",									"%u", info.limits.maxFramebufferLayers);
		myLabelText("limits.framebufferColorSampleCounts",							"%u", info.limits.framebufferColorSampleCounts);
		myLabelText("limits.framebufferDepthSampleCounts",							"%u", info.limits.framebufferDepthSampleCounts);
		myLabelText("limits.framebufferStencilSampleCounts",						"%u", info.limits.framebufferStencilSampleCounts);
		myLabelText("limits.framebufferNoAttachmentsSampleCounts",					"%u", info.limits.framebufferNoAttachmentsSampleCounts);
		myLabelText("limits.maxColorAttachments",									"%u", info.limits.maxColorAttachments);
		myLabelText("limits.sampledImageColorSampleCounts",						"%u", info.limits.sampledImageColorSampleCounts);
		myLabelText("limits.sampledImageIntegerSampleCounts",						"%u", info.limits.sampledImageIntegerSampleCounts);
		myLabelText("limits.sampledImageDepthSampleCounts",						"%u", info.limits.sampledImageDepthSampleCounts);
		myLabelText("limits.sampledImageStencilSampleCounts",						"%u", info.limits.sampledImageStencilSampleCounts);
		myLabelText("limits.storageImageSampleCounts",								"%u", info.limits.storageImageSampleCounts);
		myLabelText("limits.maxSampleMaskWords",									"%u", info.limits.maxSampleMaskWords);
		myLabelText("limits.timestampComputeAndGraphics",							"%u", info.limits.timestampComputeAndGraphics);
		myLabelText("limits.timestampPeriod",										"%f", info.limits.timestampPeriod);
		myLabelText("limits.maxClipDistances",										"%u", info.limits.maxClipDistances);
		myLabelText("limits.maxCullDistances",										"%u", info.limits.maxCullDistances);
		myLabelText("limits.maxCombinedClipAndCullDistances",						"%u", info.limits.maxCombinedClipAndCullDistances);
		myLabelText("limits.discreteQueuePriorities",								"%u", info.limits.discreteQueuePriorities);
		myLabelText("limits.pointSizeRange",										"%f, %f", info.limits.pointSizeRange[0], info.limits.pointSizeRange[1]);
		myLabelText("limits.lineWidthRange",										"%f, %f", info.limits.lineWidthRange[0], info.limits.lineWidthRange[1]);
		myLabelText("limits.pointSizeGranularity",									"%f", info.limits.pointSizeGranularity);
		myLabelText("limits.lineWidthGranularity",									"%f", info.limits.lineWidthGranularity);
		myLabelText("limits.strictLines",											"%u", info.limits.strictLines);
		myLabelText("limits.standardSampleLocations",								"%u", info.limits.standardSampleLocations);
		myLabelText("limits.optimalBufferCopyOffsetAlignment",						"%llu", info.limits.optimalBufferCopyOffsetAlignment);
		myLabelText("limits.optimalBufferCopyRowPitchAlignment",					"%llu", info.limits.optimalBufferCopyRowPitchAlignment);
		myLabelText("limits.nonCoherentAtomSize",									"%llu", info.limits.nonCoherentAtomSize);
	}


	ImGui::End();
}

} // namespace cat




