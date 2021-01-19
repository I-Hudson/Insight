#include "ispch.h"
#include "VulkanImGUIRenderer.h"
#include "Insight/Config/Config.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"

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
		auto device = VulkanDevice::Instance();
		SharedPtr<vks::VulkanRenderer> vRenderer = DynamicPointerCast<vks::VulkanRenderer>(renderer);

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

		SetupImGuiRenderPass();

		ImGui_ImplGlfw_InitForVulkan(Insight::Window::m_window, false);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vRenderer->GetInstance();
		init_info.PhysicalDevice = vRenderer->GetPhysicalDevice();
		init_info.Device = *device;
		init_info.QueueFamily = device->GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		init_info.Queue = vRenderer->GetQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = device->GetDescriptorPool();
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		init_info.ImageCount = 3;
		ImGui_ImplVulkan_Init(&init_info, vRenderer->GetPresentRenderPass());

		InitResources();
#endif
	}

	void VulkanImGUIRenderer::InitResources()
	{
#if defined(IMGUI_ENABLED)
		auto device = VulkanDevice::Instance();

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

	void VulkanImGUIRenderer::SetupImGuiRenderPass()
	{
		auto device = VulkanDevice::Instance();

		//std::array<VkAttachmentDescription, 2> attachments = {};
		//// Color attachment
		//attachments[0].format = VK_FORMAT_R8G8B8A8_SRGB;
		//attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		//attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		//attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		//attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		//attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		//// Depth attachment
		//VkFormat depthFormat;
		//vks::getSupportedDepthFormat(VulkanDevice::Instance()->GetPhysicalDevice(), &depthFormat);
		//attachments[1].format = depthFormat;
		//attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		//attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		//attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		//attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//VkAttachmentReference colorReference = {};
		//colorReference.attachment = 0;
		//colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//VkAttachmentReference depthReference = {};
		//depthReference.attachment = 1;
		//depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//VkSubpassDescription subpassDescription = {};
		//subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		//subpassDescription.colorAttachmentCount = 1;
		//subpassDescription.pColorAttachments = &colorReference;
		//subpassDescription.pDepthStencilAttachment = &depthReference;
		//subpassDescription.inputAttachmentCount = 0;
		//subpassDescription.pInputAttachments = nullptr;
		//subpassDescription.preserveAttachmentCount = 0;
		//subpassDescription.pPreserveAttachments = nullptr;
		//subpassDescription.pResolveAttachments = nullptr;

		//// Subpass dependencies for layout transitions
		//std::array<VkSubpassDependency, 2> dependencies;

		//dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		//dependencies[0].dstSubpass = 0;
		//dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		//dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		//dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		//dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		//dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		//dependencies[1].srcSubpass = 0;
		//dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		//dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		//dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		//dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		//dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		//dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		//VkRenderPassCreateInfo renderPassInfo = {};
		//renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		//renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		//renderPassInfo.pAttachments = attachments.data();
		//renderPassInfo.subpassCount = 1;
		//renderPassInfo.pSubpasses = &subpassDescription;
		//renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		//renderPassInfo.pDependencies = dependencies.data();

		//ThrowIfFailed(vkCreateRenderPass(*device, &renderPassInfo, nullptr, &renderPass));
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