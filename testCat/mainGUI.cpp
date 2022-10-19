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

MainGUI::MainGUI() : m_client(NULL), m_selectObject(NULL), m_debugButton1ClickCaller(NULL), m_debugButton1ClickFunc(NULL)
{

}

void MainGUI::init(Client* client)
{
	m_client = client;

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

void MainGUI::onGUI()
{
	if (NULL == m_client)
		return;

	_beginFrame();

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

void MainGUI::setDebugButton1ClickEvent(void* caller, ButtonClickFunc func)
{
	m_debugButton1ClickCaller = caller;
	m_debugButton1ClickFunc = func;
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

	ImGui::Checkbox("Demo window", &config.showDemoWindow);      // Edit bools storing our window open/close state

	if (NULL != m_selectObject)
		ImGui::Text(m_selectObject->name().c_str());               // Display some text (you can use a format strings too)

	if (ImGui::Button("Do pick pass"))
	{
		//printf("context menu pressed.");
		if (NULL != m_debugButton1ClickFunc)
			m_debugButton1ClickFunc(m_debugButton1ClickCaller);
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
	ImGui::LabelText("deviceType", _deviceTypeToString(info.deviceType));
	ImGui::LabelText("deviceName", info.deviceName);
	ImGui::LabelText("Vulkan API version", "%u.%u.%u", info.apiVersionMajor, info.apiVersionMinor, info.apiVersionPatch);

	// limits
	ImGui::LabelText("limits.maxUniformBufferRange",								"%u", info.limits.maxUniformBufferRange);
	ImGui::LabelText("limits.minUniformBufferOffsetAlignment",						"%llu", info.limits.minUniformBufferOffsetAlignment);
	ImGui::LabelText("limits.maxPushConstantsSize",									"%u", info.limits.maxPushConstantsSize);
	ImGui::LabelText("limits.maxBoundDescriptorSets",								"%u", info.limits.maxBoundDescriptorSets);

	// others info and limits
	if (ImGui::CollapsingHeader("Other info"))
    {
		ImGui::LabelText("driverVersion", "%u", info.driverVersion);
		ImGui::LabelText("vendorID", "%u", info.vendorID);
		ImGui::LabelText("deviceID", "%u", info.deviceID);
		string128 guid;
		for (int i = 0; i < countof(info.pipelineCacheUUID); ++i)
			guid.format_append("%x:", info.pipelineCacheUUID[i]);
		ImGui::LabelText("pipelineCacheUUID", guid.c_str());

		// limits
		ImGui::LabelText("limits.maxImageDimension1D",									"%u", info.limits.maxImageDimension1D);
		ImGui::LabelText("limits.maxImageDimension2D",									"%u", info.limits.maxImageDimension2D);
		ImGui::LabelText("limits.maxImageDimension3D",									"%u", info.limits.maxImageDimension3D);
		ImGui::LabelText("limits.maxImageDimensionCube",								"%u", info.limits.maxImageDimensionCube);
		ImGui::LabelText("limits.maxImageArrayLayers",									"%u", info.limits.maxImageArrayLayers);
		ImGui::LabelText("limits.maxTexelBufferElements",								"%u", info.limits.maxTexelBufferElements);
		//ImGui::LabelText("limits.maxUniformBufferRange",								"%u", info.limits.maxUniformBufferRange);
		ImGui::LabelText("limits.maxStorageBufferRange",								"%u", info.limits.maxStorageBufferRange);
		//ImGui::LabelText("limits.maxPushConstantsSize",									"%u", info.limits.maxPushConstantsSize);
		ImGui::LabelText("limits.maxMemoryAllocationCount",								"%u", info.limits.maxMemoryAllocationCount);
		ImGui::LabelText("limits.maxSamplerAllocationCount",							"%u", info.limits.maxSamplerAllocationCount);
		ImGui::LabelText("limits.bufferImageGranularity",								"%llu", info.limits.bufferImageGranularity);
		ImGui::LabelText("limits.sparseAddressSpaceSize",								"%llu", info.limits.sparseAddressSpaceSize);
		//ImGui::LabelText("limits.maxBoundDescriptorSets",								"%u", info.limits.maxBoundDescriptorSets);
		ImGui::LabelText("limits.maxPerStageDescriptorSamplers",						"%u", info.limits.maxPerStageDescriptorSamplers);
		ImGui::LabelText("limits.maxPerStageDescriptorUniformBuffers",					"%u", info.limits.maxPerStageDescriptorUniformBuffers);
		ImGui::LabelText("limits.maxPerStageDescriptorStorageBuffers",					"%u", info.limits.maxPerStageDescriptorStorageBuffers);
		ImGui::LabelText("limits.maxPerStageDescriptorSampledImages",					"%u", info.limits.maxPerStageDescriptorSampledImages);
		ImGui::LabelText("limits.maxPerStageDescriptorStorageImages",					"%u", info.limits.maxPerStageDescriptorStorageImages);
		ImGui::LabelText("limits.maxPerStageDescriptorInputAttachments",				"%u", info.limits.maxPerStageDescriptorInputAttachments);
		ImGui::LabelText("limits.maxPerStageResources",									"%u", info.limits.maxPerStageResources);
		ImGui::LabelText("limits.maxDescriptorSetSamplers",								"%u", info.limits.maxDescriptorSetSamplers);
		ImGui::LabelText("limits.maxDescriptorSetUniformBuffers",						"%u", info.limits.maxDescriptorSetUniformBuffers);
		ImGui::LabelText("limits.maxDescriptorSetUniformBuffersDynamic",				"%u", info.limits.maxDescriptorSetUniformBuffersDynamic);
		ImGui::LabelText("limits.maxDescriptorSetStorageBuffers",						"%u", info.limits.maxDescriptorSetStorageBuffers);
		ImGui::LabelText("limits.maxDescriptorSetStorageBuffersDynamic",				"%u", info.limits.maxDescriptorSetStorageBuffersDynamic);
		ImGui::LabelText("limits.maxDescriptorSetSampledImages",						"%u", info.limits.maxDescriptorSetSampledImages);
		ImGui::LabelText("limits.maxDescriptorSetStorageImages",						"%u", info.limits.maxDescriptorSetStorageImages);
		ImGui::LabelText("limits.maxDescriptorSetInputAttachments",						"%u", info.limits.maxDescriptorSetInputAttachments);
		ImGui::LabelText("limits.maxVertexInputAttributes",								"%u", info.limits.maxVertexInputAttributes);
		ImGui::LabelText("limits.maxVertexInputBindings",								"%u", info.limits.maxVertexInputBindings);
		ImGui::LabelText("limits.maxVertexInputAttributeOffset",						"%u", info.limits.maxVertexInputAttributeOffset);
		ImGui::LabelText("limits.maxVertexInputBindingStride",							"%u", info.limits.maxVertexInputBindingStride);
		ImGui::LabelText("limits.maxVertexOutputComponents",							"%u", info.limits.maxVertexOutputComponents);
		ImGui::LabelText("limits.maxTessellationGenerationLevel",						"%u", info.limits.maxTessellationGenerationLevel);
		ImGui::LabelText("limits.maxTessellationPatchSize",								"%u", info.limits.maxTessellationPatchSize);
		ImGui::LabelText("limits.maxTessellationControlPerVertexInputComponents",		"%u", info.limits.maxTessellationControlPerVertexInputComponents);
		ImGui::LabelText("limits.maxTessellationControlPerVertexOutputComponents",		"%u", info.limits.maxTessellationControlPerVertexOutputComponents);
		ImGui::LabelText("limits.maxTessellationControlPerPatchOutputComponents",		"%u", info.limits.maxTessellationControlPerPatchOutputComponents);
		ImGui::LabelText("limits.maxTessellationControlTotalOutputComponents",			"%u", info.limits.maxTessellationControlTotalOutputComponents);
		ImGui::LabelText("limits.maxTessellationEvaluationInputComponents",				"%u", info.limits.maxTessellationEvaluationInputComponents);
		ImGui::LabelText("limits.maxTessellationEvaluationOutputComponents",			"%u", info.limits.maxTessellationEvaluationOutputComponents);
		ImGui::LabelText("limits.maxGeometryShaderInvocations",							"%u", info.limits.maxGeometryShaderInvocations);
		ImGui::LabelText("limits.maxGeometryInputComponents",							"%u", info.limits.maxGeometryInputComponents);
		ImGui::LabelText("limits.maxGeometryOutputComponents",							"%u", info.limits.maxGeometryOutputComponents);
		ImGui::LabelText("limits.maxGeometryOutputVertices",							"%u", info.limits.maxGeometryOutputVertices);
		ImGui::LabelText("limits.maxGeometryTotalOutputComponents",						"%u", info.limits.maxGeometryTotalOutputComponents);
		ImGui::LabelText("limits.maxFragmentInputComponents",							"%u", info.limits.maxFragmentInputComponents);
		ImGui::LabelText("limits.maxFragmentOutputAttachments",							"%u", info.limits.maxFragmentOutputAttachments);
		ImGui::LabelText("limits.maxFragmentDualSrcAttachments",						"%u", info.limits.maxFragmentDualSrcAttachments);
		ImGui::LabelText("limits.maxFragmentCombinedOutputResources",					"%u", info.limits.maxFragmentCombinedOutputResources);
		ImGui::LabelText("limits.maxComputeSharedMemorySize",							"%u", info.limits.maxComputeSharedMemorySize);
		ImGui::LabelText("limits.maxComputeWorkGroupCount",								"%u, %u, %u", info.limits.maxComputeWorkGroupCount[0], info.limits.maxComputeWorkGroupCount[1], info.limits.maxComputeWorkGroupCount[2]);
		ImGui::LabelText("limits.maxComputeWorkGroupInvocations",						"%u", info.limits.maxComputeWorkGroupInvocations);
		ImGui::LabelText("limits.maxComputeWorkGroupSize",								"%u, %u, %u", info.limits.maxComputeWorkGroupSize[0], info.limits.maxComputeWorkGroupSize[1], info.limits.maxComputeWorkGroupSize[2]);
		ImGui::LabelText("limits.subPixelPrecisionBits",								"%u", info.limits.subPixelPrecisionBits);
		ImGui::LabelText("limits.subTexelPrecisionBits",								"%u", info.limits.subTexelPrecisionBits);
		ImGui::LabelText("limits.mipmapPrecisionBits",									"%u", info.limits.mipmapPrecisionBits);
		ImGui::LabelText("limits.maxDrawIndexedIndexValue",								"%u", info.limits.maxDrawIndexedIndexValue);
		ImGui::LabelText("limits.maxDrawIndirectCount",									"%u", info.limits.maxDrawIndirectCount);
		ImGui::LabelText("limits.maxSamplerLodBias",									"%f", info.limits.maxSamplerLodBias);
		ImGui::LabelText("limits.maxSamplerAnisotropy",									"%f", info.limits.maxSamplerAnisotropy);
		ImGui::LabelText("limits.maxViewports",											"%u", info.limits.maxViewports);
		ImGui::LabelText("limits.maxViewportDimensions",								"%u, %u", info.limits.maxViewportDimensions[0], info.limits.maxViewportDimensions[1]);
		ImGui::LabelText("limits.viewportBoundsRange",									"%f, %f", info.limits.viewportBoundsRange[0], info.limits.maxViewportDimensions[1]);
		ImGui::LabelText("limits.viewportSubPixelBits",									"%u", info.limits.viewportSubPixelBits);
		ImGui::LabelText("limits.minMemoryMapAlignment",								"%llu", info.limits.minMemoryMapAlignment);
		ImGui::LabelText("limits.minTexelBufferOffsetAlignment",						"%llu", info.limits.minTexelBufferOffsetAlignment);
		//ImGui::LabelText("limits.minUniformBufferOffsetAlignment",						"%llu", info.limits.minUniformBufferOffsetAlignment);
		ImGui::LabelText("limits.minStorageBufferOffsetAlignment",						"%llu", info.limits.minStorageBufferOffsetAlignment);
		ImGui::LabelText("limits.minTexelOffset",										"%d", info.limits.minTexelOffset);
		ImGui::LabelText("limits.maxTexelOffset",										"%u", info.limits.maxTexelOffset);
		ImGui::LabelText("limits.minTexelGatherOffset",									"%d", info.limits.minTexelGatherOffset);
		ImGui::LabelText("limits.maxTexelGatherOffset",									"%u", info.limits.maxTexelGatherOffset);
		ImGui::LabelText("limits.minInterpolationOffset",								"%f", info.limits.minInterpolationOffset);
		ImGui::LabelText("limits.maxInterpolationOffset",								"%f", info.limits.maxInterpolationOffset);
		ImGui::LabelText("limits.subPixelInterpolationOffsetBits",						"%u", info.limits.subPixelInterpolationOffsetBits);
		ImGui::LabelText("limits.maxFramebufferWidth",									"%u", info.limits.maxFramebufferWidth);
		ImGui::LabelText("limits.maxFramebufferHeight",									"%u", info.limits.maxFramebufferHeight);
		ImGui::LabelText("limits.maxFramebufferLayers",									"%u", info.limits.maxFramebufferLayers);
		ImGui::LabelText("limits.framebufferColorSampleCounts",							"%u", info.limits.framebufferColorSampleCounts);
		ImGui::LabelText("limits.framebufferDepthSampleCounts",							"%u", info.limits.framebufferDepthSampleCounts);
		ImGui::LabelText("limits.framebufferStencilSampleCounts",						"%u", info.limits.framebufferStencilSampleCounts);
		ImGui::LabelText("limits.framebufferNoAttachmentsSampleCounts",					"%u", info.limits.framebufferNoAttachmentsSampleCounts);
		ImGui::LabelText("limits.maxColorAttachments",									"%u", info.limits.maxColorAttachments);
		ImGui::LabelText("limits.sampledImageColorSampleCounts",						"%u", info.limits.sampledImageColorSampleCounts);
		ImGui::LabelText("limits.sampledImageIntegerSampleCounts",						"%u", info.limits.sampledImageIntegerSampleCounts);
		ImGui::LabelText("limits.sampledImageDepthSampleCounts",						"%u", info.limits.sampledImageDepthSampleCounts);
		ImGui::LabelText("limits.sampledImageStencilSampleCounts",						"%u", info.limits.sampledImageStencilSampleCounts);
		ImGui::LabelText("limits.storageImageSampleCounts",								"%u", info.limits.storageImageSampleCounts);
		ImGui::LabelText("limits.maxSampleMaskWords",									"%u", info.limits.maxSampleMaskWords);
		ImGui::LabelText("limits.timestampComputeAndGraphics",							"%u", info.limits.timestampComputeAndGraphics);
		ImGui::LabelText("limits.timestampPeriod",										"%f", info.limits.timestampPeriod);
		ImGui::LabelText("limits.maxClipDistances",										"%u", info.limits.maxClipDistances);
		ImGui::LabelText("limits.maxCullDistances",										"%u", info.limits.maxCullDistances);
		ImGui::LabelText("limits.maxCombinedClipAndCullDistances",						"%u", info.limits.maxCombinedClipAndCullDistances);
		ImGui::LabelText("limits.discreteQueuePriorities",								"%u", info.limits.discreteQueuePriorities);
		ImGui::LabelText("limits.pointSizeRange",										"%f, %f", info.limits.pointSizeRange[0], info.limits.pointSizeRange[1]);
		ImGui::LabelText("limits.lineWidthRange",										"%f, %f", info.limits.lineWidthRange[0], info.limits.lineWidthRange[1]);
		ImGui::LabelText("limits.pointSizeGranularity",									"%f", info.limits.pointSizeGranularity);
		ImGui::LabelText("limits.lineWidthGranularity",									"%f", info.limits.lineWidthGranularity);
		ImGui::LabelText("limits.strictLines",											"%u", info.limits.strictLines);
		ImGui::LabelText("limits.standardSampleLocations",								"%u", info.limits.standardSampleLocations);
		ImGui::LabelText("limits.optimalBufferCopyOffsetAlignment",						"%llu", info.limits.optimalBufferCopyOffsetAlignment);
		ImGui::LabelText("limits.optimalBufferCopyRowPitchAlignment",					"%llu", info.limits.optimalBufferCopyRowPitchAlignment);
		ImGui::LabelText("limits.nonCoherentAtomSize",									"%llu", info.limits.nonCoherentAtomSize);
	}


	ImGui::End();
}

} // namespace cat




