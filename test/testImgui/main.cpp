// Dear ImGui: standalone example application for Glfw + Vulkan
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_vulkan.h"
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
//#define GLFW_INCLUDE_NONE
//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <Windows.h>
#include "cat/simpleVulkan.h"
#include "scl/time.h"  // 改用Windows的Sleep函数

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

static VkAllocationCallbacks*   g_Allocator = NULL;
static VkInstance               g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily = (uint32_t)-1;
static VkQueue                  g_Queue = VK_NULL_HANDLE;
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;
svkDevice		g_svkDevice;
svkSwapchain	g_svkSwapchain;
svkSurface		g_svkSurface;
svkFrame		g_svkFrames[4];
int				g_frameCount = 0;
VkRenderPass	g_renderPass = VK_NULL_HANDLE;
svkImage		g_depthImage;
HINSTANCE g_instance;
HWND g_hwnd;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static int                      g_MinImageCount = 3;
static bool                     g_SwapChainRebuild = false;

void copySwapchainToMainWindowData(ImGui_ImplVulkanH_Window* wd, svkSwapchain& swapchain, svkSurface& surface);

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

void recreate_swapchain()
{
	vkDeviceWaitIdle(g_svkDevice.device); // FIXME: We could wait on the Queue if we had the queue in wd-> (otherwise VulkanH functions can't use globals)
														  //ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
														  //ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
	
	// Destroy old frames
	svkDestroyFrames(g_svkDevice, g_svkFrames, g_frameCount);
	
	svkDestroySwapchain(g_svkDevice, g_svkSwapchain);
	svkSwapchain oldswapchain = { NULL };
	svkRefreshSurfaceSize(g_svkDevice, g_svkSurface);

	//char log[1024] = { 0 };
	//sprintf(log, "wm_size.width = %d, height = %d, surface.width = %d, height = %d\n", width, height, g_svkSurface.width, g_svkSurface.height);
	//OutputDebugStringA(log);

	// Recreate depth image with new size
	svkDestroyImage(g_svkDevice, g_depthImage);
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
	g_depthImage = svkCreateAttachmentDepthImage(g_svkDevice, depthFormat, g_svkSurface.width, g_svkSurface.height);

	g_svkSwapchain = svkCreateSwapchain(g_svkDevice, g_svkSurface, oldswapchain);
	
	// Recreate frames
	g_frameCount = svkCreateFrames(g_svkDevice, g_svkSwapchain, g_depthImage.imageView, g_renderPass, g_svkSurface.width, g_svkSurface.height, g_svkFrames, 4);
	
	copySwapchainToMainWindowData(&g_MainWindowData, g_svkSwapchain, g_svkSurface);

	g_MainWindowData.FrameIndex = 0;
	g_SwapChainRebuild = false;
}

void on_present_failed(void* userData, VkResult result)
{
	printf("present failed : result = %d\n", result);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreate_swapchain();
	}
	else if (VK_SUBOPTIMAL_KHR == result)
	{
		// demo->swapchain is not as optimal as it could be, but the platform's
		// presentation engine will still present the image correctly.
	}
	else if (VK_ERROR_SURFACE_LOST_KHR == result)
	{
		svkDestroySurface(g_Instance, g_svkDevice, g_svkSurface);
		g_svkSurface = svkCreateSurface(g_Instance, g_svkDevice, g_instance, g_hwnd);

		recreate_swapchain();
	}
}

static void CleanupVulkan()
{
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
	if (NULL != vkDestroyDebugReportCallbackEXT)
		vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);
}

static void CleanupVulkanWindow()
{
	// Cleanup our frames
	svkDestroyFrames(g_svkDevice, g_svkFrames, g_frameCount);
	
	// Cleanup depth image
	svkDestroyImage(g_svkDevice, g_depthImage);
	
	// Cleanup render pass
	if (g_renderPass != VK_NULL_HANDLE)
		svkDestroyRenderPass(g_svkDevice, g_renderPass);
	
	// Cleanup swapchain
	svkDestroySwapchain(g_svkDevice, g_svkSwapchain);
	
	// Cleanup surface
	svkDestroySurface(g_Instance, g_svkDevice, g_svkSurface);
	
    // CRITICAL: Cannot call ImGui_ImplVulkanH_DestroyWindow() here!
    // Reason: All Vulkan resources (Fences, CommandBuffers, Semaphores) are managed by svk system
    // and have already been destroyed by svkDestroyFrames() above.
    // ImGui_ImplVulkanH_DestroyWindow() would attempt to destroy already-freed resources, causing crashes.
    // The actual resource destruction occurs in svkDestroyFrames() at simplevulkan.cpp:1014-1021
    // ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData, g_Allocator);
    
    // Only free ImGui's allocated memory structures, do not touch any Vulkan resources
    if (g_MainWindowData.Frames)
    {
        IM_FREE(g_MainWindowData.Frames);
        g_MainWindowData.Frames = nullptr;
    }
    if (g_MainWindowData.FrameSemaphores)
    {
        IM_FREE(g_MainWindowData.FrameSemaphores);
        g_MainWindowData.FrameSemaphores = nullptr;
    }
}

static void My_FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data, int frameIndex)
{
	svkFrame& frame = g_svkFrames[frameIndex];
	VkCommandBuffer cmd_buf = frame.commandBuffer;
	
	svkBeginCommandBuffer(cmd_buf);
	svkCmdBeginRenderPass(cmd_buf, 0.2f, 0.2f, 0.2f, 0.2f, 1.0f, 0, g_renderPass, frame.framebuffer, g_svkSurface.width, g_svkSurface.height, false);
	
	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(draw_data, cmd_buf);
	
	vkCmdEndRenderPass(cmd_buf);
	VkResult err = svkEndCommandBuffer(cmd_buf);
	check_vk_result(err);
}

int g_frame_index = 0;

static void My_FramePresent(ImGui_ImplVulkanH_Window* wd, int frameIndex)
{
	svkFrame& frame = g_svkFrames[frameIndex];
	
	// Submit command buffer
	svkQueueSubmit(g_svkDevice, &frame.commandBuffer, 1, &frame.imageAcquireSemaphore, &frame.drawCompleteSemaphore, frame.fence);
	
	// Present frame
	svkPresent(g_svkDevice, g_svkSwapchain, g_svkFrames, frameIndex, NULL, on_present_failed);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void copySwapchainToMainWindowData(ImGui_ImplVulkanH_Window* wd, svkSwapchain& swapchain, svkSurface& surface)
{
	wd->Swapchain = swapchain.swapchain;

	wd->SurfaceFormat.format = swapchain.format;
	wd->SurfaceFormat.colorSpace = swapchain.colorSpace;
	wd->PresentMode = VK_PRESENT_MODE_FIFO_KHR;
	wd->RenderPass = g_renderPass;
	wd->ImageCount = g_frameCount;
	wd->Frames = (ImGui_ImplVulkanH_Frame*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_Frame) * wd->ImageCount);
	wd->FrameSemaphores = (ImGui_ImplVulkanH_FrameSemaphores*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_FrameSemaphores) * wd->ImageCount);
	for (int i = 0; i < g_frameCount; ++i)
	{
		wd->Frames[i].BackbufferView = g_svkFrames[i].imageView;
		wd->Frames[i].Backbuffer = NULL;
		wd->Frames[i].CommandBuffer = g_svkFrames[i].commandBuffer;
		wd->Frames[i].CommandPool = g_svkDevice.commandPool;
		wd->Frames[i].Framebuffer = g_svkFrames[i].framebuffer;
		wd->Frames[i].Fence = g_svkFrames[i].fence;
		wd->FrameSemaphores[i].ImageAcquiredSemaphore = g_svkFrames[i].imageAcquireSemaphore;
		wd->FrameSemaphores[i].RenderCompleteSemaphore = g_svkFrames[i].drawCompleteSemaphore;
	}
}

int main()
{
	VkResult err;
	memset(&g_svkDevice, 0, sizeof(g_svkDevice));
	memset(&g_svkSurface, 0, sizeof(g_svkSurface));
	memset(&g_svkSwapchain, 0, sizeof(g_svkSwapchain));
	memset(&g_svkFrames, 0, sizeof(g_svkFrames));
	memset(&g_depthImage, 0, sizeof(g_depthImage));
	g_frameCount = 0;
	g_renderPass = VK_NULL_HANDLE;

	ImGui_ImplWin32_EnableDpiAwareness();
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindow(wc.lpszClassName, L"Dear ImGui DirectX12 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);
	g_hwnd = hwnd;
	g_instance = wc.hInstance;

	g_Instance			= svkCreateInstance(true);
	g_svkDevice			= svkCreateDevice(g_Instance);
	g_Device			= g_svkDevice.device;
	g_PhysicalDevice	= g_svkDevice.gpu;
	g_Queue				= g_svkDevice.queue;
	g_QueueFamily		= g_svkDevice.queueFamilyIndex;

	int countPerType[VK_DESCRIPTOR_TYPE_RANGE_SIZE] = { 0 };
	for (int i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i)
		countPerType[i] = 1000;
	g_DescriptorPool = svkCreateDescriptorPoolEx2(g_svkDevice, 1000, countPerType);

	unsigned int queueCount = 16;
	VkQueueFamilyProperties queues[16];
	vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &queueCount, queues);

	g_svkSurface = svkCreateSurface(g_Instance, g_svkDevice, wc.hInstance, hwnd);

	int w = g_svkSurface.width;
	int h = g_svkSurface.height;
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
	wd->Surface = g_svkSurface.surface;

	// Create render pass
	VkFormat colorFormat = g_svkSurface.capabilities.currentExtent.width > 0 ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_B8G8R8A8_UNORM;
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
	g_renderPass = svkCreateRenderPass(g_svkDevice, colorFormat, depthFormat, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	// Create depth image
	g_depthImage = svkCreateAttachmentDepthImage(g_svkDevice, depthFormat, w, h);

	svkSwapchain oldSwapchain;
	oldSwapchain.swapchain = NULL;
	g_svkSwapchain = svkCreateSwapchain(g_svkDevice, g_svkSurface, oldSwapchain);
	
	// Create frames
	g_frameCount = svkCreateFrames(g_svkDevice, g_svkSwapchain, g_depthImage.imageView, g_renderPass, w, h, g_svkFrames, 4);
	
	copySwapchainToMainWindowData(wd, g_svkSwapchain, g_svkSurface);


	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = g_Allocator;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

    io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("d:/imgui/misc/fonts/msyh.ttc", 24.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    IM_ASSERT(font != NULL);

    // Upload Fonts
    {
		VkCommandBuffer commandBuffer = svkAllocCommandBuffer(g_svkDevice);
		svkBeginCommandBuffer(commandBuffer, true);

        //// Use any command queue
        //VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
        //VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

        //err = vkResetCommandPool(g_Device, command_pool, 0);
        //check_vk_result(err);
        //VkCommandBufferBeginInfo begin_info = {};
        //begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        //err = vkBeginCommandBuffer(command_buffer, &begin_info);
        //check_vk_result(err);

        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		svkEndCommandBufferAndSubmit(g_svkDevice, commandBuffer);

        //VkSubmitInfo end_info = {};
        //end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        //end_info.commandBufferCount = 1;
        //end_info.pCommandBuffers = &command_buffer;
        //err = vkEndCommandBuffer(command_buffer);
        //check_vk_result(err);
        //err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
        //check_vk_result(err);

        //err = vkDeviceWaitIdle(g_Device);
        //check_vk_result(err);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    //while (!glfwWindowShouldClose(window))
	// Main loop
	bool done = false;
	int current_frame_index = 0;
	while (!done)
    {
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            wd->ClearValue.color.float32[3] = clear_color.w;
            
            // Acquire next frame
            int frame_index = svkAcquireNextImage(g_svkDevice, g_svkSwapchain, g_svkFrames, current_frame_index, NULL, on_present_failed);
            
            //FrameRender(wd, draw_data);
            My_FrameRender(wd, draw_data, frame_index);

            //FramePresent(wd);
			My_FramePresent(wd, frame_index);
            
            current_frame_index = (current_frame_index + 1) % g_frameCount;

        }
		Sleep(1);  // Windows Sleep函数，参数为毫秒
    }

    // Cleanup
    err = vkDeviceWaitIdle(g_Device);
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    //ImGui_ImplGlfw_Shutdown();
	ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupVulkanWindow();
    CleanupVulkan();

    //glfwDestroyWindow(window);
    //glfwTerminate();

    return 0;
}



// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		{
			int width = (UINT)LOWORD(lParam);
			int height = (UINT)HIWORD(lParam);

			//break;
			//if (width > 0 && height > 0)
			//{
			//	if (NULL == ImGui::GetCurrentContext())
			//		break;
			//	vkDeviceWaitIdle(g_svkDevice.device); // FIXME: We could wait on the Queue if we had the queue in wd-> (otherwise VulkanH functions can't use globals)
			//	//ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
			//	//ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
			//	svkDestroySwapchain(g_svkDevice, g_svkSwapchain);
			//	svkSwapchain oldswapchain = { NULL };
			//	svkRefreshSurfaceSize(g_svkDevice, g_svkSurface);

			//	char log[1024] = { 0 };
			//	sprintf(log, "wm_size.width = %d, height = %d, surface.width = %d, height = %d\n", width, height, g_svkSurface.width, g_svkSurface.height);
			//	OutputDebugStringA(log);

			//	g_svkSwapchain = svkCreateSwapchain(g_svkDevice, g_svkSurface, oldswapchain, false);
			//	copySwapchainToMainWindowData(&g_MainWindowData, g_svkSwapchain, g_svkSurface);
			//	
			//	g_MainWindowData.FrameIndex = 0;
			//	g_SwapChainRebuild = false;
			//}
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

