#include "ispch.h"
#include "VulkanImGUIRenderer.h"
#include "Insight/Config/Config.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "Insight/Module/WindowModule.h"
#include "IconsFontAwesome5.h"

#include "Insight/Input/Input.h"
#include "Insight/Time/Time.h"
#include "Insight/Event/EventManager.h"

namespace vks
{
	VulkanImGUIRenderer::VulkanImGUIRenderer()
	{
		REG_EVENT_HANDLE(Insight::EventType::VulkanWindowResize, VulkanImGUIRenderer::WindowResize);
	}

	VulkanImGUIRenderer::~VulkanImGUIRenderer()
	{
#if defined(IMGUI_ENABLED)
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
#endif
	}

	void VulkanImGUIRenderer::NewFrame()
	{
		IS_PROFILE_FUNCTION();
#if defined(IMGUI_ENABLED)
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//ImGui::DockSpaceOverViewport();
#endif
	}

	void VulkanImGUIRenderer::EndFrame()
	{
		IS_PROFILE_FUNCTION();
#if defined(IMGUI_ENABLED)
		ImGui::Render();
		ImGui::UpdatePlatformWindows();
#endif
	}

	void VulkanImGUIRenderer::Render(VkCommandBuffer commandBuffer)
	{
		IS_PROFILE_FUNCTION();
#if defined(IMGUI_ENABLED)
		//OPTICK_GPU_EVENT("ImGui Draw");
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
#endif
	}

	void VulkanImGUIRenderer::Init(SharedPtr<Insight::Renderer> renderer)
	{
#if defined(IMGUI_ENABLED)
		m_renderer = DynamicPointerCast<vks::VulkanRenderer>(renderer);
		device = m_renderer->GetDevice();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplGlfw_InitForVulkan(Insight::Window::m_window, false);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = m_renderer->GetInstance();
		init_info.PhysicalDevice = m_renderer->GetPhysicalDevice();
		init_info.Device = *m_renderer->GetDevice();
		init_info.QueueFamily = device->GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		init_info.Queue = m_renderer->GetQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = device->GetDescriptorPool();
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		init_info.ImageCount = 3;
		ImGui_ImplVulkan_Init(&init_info, device->GetRenderPass());

		InitResources();
#endif
	}

	void VulkanImGUIRenderer::InitResources()
	{
#if defined(IMGUI_ENABLED)
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontFromFileTTF("./data/fonts/montserrat/Montserrat-Bold.ttf", 18.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("./data/fonts/montserrat/Montserrat-Regular.ttf", 18.0f);

		// merge in icons from Font Awesome
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
		io.Fonts->AddFontFromFileTTF("./data/fonts/" FONT_ICON_FILE_NAME_FAS, 16.0f, &icons_config, icons_ranges);
		// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

		VkCommandBuffer command_buffer = device->CreateSingleUseBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
		device->FlushCommandBuffer(command_buffer, device->GetQueue(VK_QUEUE_GRAPHICS_BIT));
#endif
	}

	void VulkanImGUIRenderer::WindowResize(Insight::Event const& event)
	{
#if defined(IMGUI_ENABLED)
		Insight::VulkanResizeEvent resizeEvent = static_cast<const Insight::VulkanResizeEvent&>(event);

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)resizeEvent.m_width, (float)resizeEvent.m_height);
		ImGui_ImplVulkan_SetMinImageCount(2);
#endif
	}
}