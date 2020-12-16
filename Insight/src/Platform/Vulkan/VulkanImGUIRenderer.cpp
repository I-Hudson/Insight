#include "ispch.h"
#include "VulkanImGUIRenderer.h"
#include "Insight/Config/Config.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "Insight/Module/WindowModule.h"

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
		SetStyle();
#endif
	}

	void VulkanImGUIRenderer::InitResources()
	{
#if defined(IMGUI_ENABLED)
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontFromFileTTF("./data/fonts/montserrat/Montserrat-Bold.ttf", 18.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("./data/fonts/montserrat/Montserrat-Regular.ttf", 18.0f);

		VkCommandBuffer command_buffer = device->CreateSingleUseBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
		device->FlushCommandBuffer(command_buffer, device->GetQueue(VK_QUEUE_GRAPHICS_BIT));
#endif
	}

	void VulkanImGUIRenderer::SetStyle()
	{
		auto& style = ImGui::GetStyle().Colors;

		glm::vec4 windowBg = CONFIG_VAL(Insight::Config::EditorConfig.WindowBG);
		style[ImGuiCol_WindowBg] = ImVec4(windowBg.x, windowBg.y, windowBg.z, windowBg.w);

		glm::vec4 header = CONFIG_VAL(Insight::Config::EditorConfig.Header);
		style[ImGuiCol_Header] = ImVec4(header.x, header.y, header.z, header.w);
		glm::vec4 headerHovered = CONFIG_VAL(Insight::Config::EditorConfig.HeaderHovered);
		style[ImGuiCol_HeaderHovered] = ImVec4(headerHovered.x, headerHovered.y, headerHovered.z, headerHovered.w);
		glm::vec4 headerActive = CONFIG_VAL(Insight::Config::EditorConfig.HeaderActive);
		style[ImGuiCol_HeaderActive] = ImVec4(headerActive.x, headerActive.y, headerActive.z, headerActive.w);

		glm::vec4 button = CONFIG_VAL(Insight::Config::EditorConfig.Button);
		style[ImGuiCol_Button] = ImVec4(button.x, button.y, button.z, button.w);
		glm::vec4 buttonHovered = CONFIG_VAL(Insight::Config::EditorConfig.ButtonHovered);
		style[ImGuiCol_ButtonHovered] = ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, buttonHovered.w);
		glm::vec4 buttonActive = CONFIG_VAL(Insight::Config::EditorConfig.ButtonActive);
		style[ImGuiCol_ButtonActive] = ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, buttonActive.w);

		glm::vec4 frame = CONFIG_VAL(Insight::Config::EditorConfig.FrameBg);
		style[ImGuiCol_FrameBg] = ImVec4(frame.x, frame.y, frame.z, frame.w);
		glm::vec4 frameBg = CONFIG_VAL(Insight::Config::EditorConfig.FrameBgHovered);
		style[ImGuiCol_FrameBgHovered] = ImVec4(frameBg.x, frameBg.y, frameBg.z, frameBg.w);
		glm::vec4 frameBgActive = CONFIG_VAL(Insight::Config::EditorConfig.FrameBgActive);
		style[ImGuiCol_FrameBgActive] = ImVec4(frameBgActive.x, frameBgActive.y, frameBgActive.z, frameBgActive.w);

		glm::vec4 tab = CONFIG_VAL(Insight::Config::EditorConfig.Tab);
		style[ImGuiCol_Tab] = ImVec4(tab.x, tab.y, tab.z, tab.w);
		glm::vec4 tabHovered = CONFIG_VAL(Insight::Config::EditorConfig.TabHovered);
		style[ImGuiCol_TabHovered] = ImVec4(tabHovered.x, tabHovered.y, tabHovered.z, tabHovered.w);
		glm::vec4 tabActive = CONFIG_VAL(Insight::Config::EditorConfig.TabActive);
		style[ImGuiCol_TabActive] = ImVec4(tabActive.x, tabActive.y, tabActive.z, tabActive.w);
		glm::vec4 tabUnfocused = CONFIG_VAL(Insight::Config::EditorConfig.TabUnfocused);
		style[ImGuiCol_TabUnfocused] = ImVec4(tabUnfocused.x, tabUnfocused.y, tabUnfocused.z, tabUnfocused.w);
		glm::vec4 tabUnfocusedActive = CONFIG_VAL(Insight::Config::EditorConfig.TabUnfocusedActive);
		style[ImGuiCol_TabUnfocusedActive] = ImVec4(tabUnfocusedActive.x, tabUnfocusedActive.y, tabUnfocusedActive.z, tabUnfocusedActive.w);

		glm::vec4 titleBg = CONFIG_VAL(Insight::Config::EditorConfig.TitleBg);
		style[ImGuiCol_TitleBg] = ImVec4(titleBg.x, titleBg.y, titleBg.z, titleBg.w);
		glm::vec4 titleBgActive = CONFIG_VAL(Insight::Config::EditorConfig.TitleBgActive);
		style[ImGuiCol_TitleBgActive] = ImVec4(titleBgActive.x, titleBgActive.y, titleBgActive.z, titleBgActive.w);
		glm::vec4 titleBgCollapsed = CONFIG_VAL(Insight::Config::EditorConfig.TitleBgCollapsed);
		style[ImGuiCol_TitleBgCollapsed] = ImVec4(titleBgCollapsed.x, titleBgCollapsed.y, titleBgCollapsed.z, titleBgCollapsed.w);
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