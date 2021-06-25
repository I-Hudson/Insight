
#include "Engine/GraphicsAPI/Vulkan/VulkanImGUIRenderer.h"
#include "Engine/Config/Config.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUAdapterVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/RenderGraph/RenderGraphVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUImageVulkan.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "ImGuizmo.h"
#include "IconsFontAwesome5.h"

#include "Engine/Input/Input.h"
#include "Engine/Time/Time.h"
#include "Engine/Event/EventManager.h"

namespace Insight::GraphicsAPI::Vulkan
{
	VulkanImGUIRenderer::VulkanImGUIRenderer()
	{
		REG_EVENT_HANDLE(EventType::VulkanWindowResize, VulkanImGUIRenderer::WindowResize);
	
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
	}

	VulkanImGUIRenderer::~VulkanImGUIRenderer()
	{
#if defined(IMGUI_ENABLED)
		GPUDeviceVulkan& device = static_cast<GPUDeviceVulkan&>(*Graphics::GPUDevice::Instance());
		if (m_init)
		{
			vkDestroyDescriptorPool(device.Device, m_descriptorPool, nullptr);

			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}
#endif
	}

	void VulkanImGUIRenderer::NewFrame()
	{
		IS_PROFILE_FUNCTION();
#if defined(IMGUI_ENABLED)
		if (m_init)
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();
			ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
		}
#endif
	}

	void VulkanImGUIRenderer::EndFrame()
	{
		IS_PROFILE_FUNCTION();
#if defined(IMGUI_ENABLED)
		if (m_init)
		{
			ImGui::Render();
			ImGui::UpdatePlatformWindows();
		}
#endif
	}

	void VulkanImGUIRenderer::Render()
	{
		IS_PROFILE_FUNCTION();
#if defined(IMGUI_ENABLED)
		Graphics::RenderGraph& graph = *Graphics::RenderGraph::Instance();
		auto& imguiPass = graph.AddPass("ImGui", Graphics::RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
		imguiPass.AddColorOutput(graph.GetBackbufferSourceName());
		imguiPass.SetPassQueue(Graphics::RenderPassQueue::UI);

		imguiPass.OnBeginRender([this](Graphics::RenderPass* pass, Graphics::GPURenderGraphPass* graphPass)
		{
			if (!m_init)
			{
				m_renderpass = static_cast<GPURenderGraphPassVulkan*>(graphPass)->GetRenderPassVulkan();
				Init();
			}
		});

		//OPTICK_GPU_EVENT("ImGui Draw");
		imguiPass.SetRenderFunc([this](Graphics::GPUCommandBuffer* cmdBuffer, Graphics::FrameBufferResources& buffers, Graphics::GPUDescriptorBuilder* builder, Graphics::RenderPass& pass, Graphics::RenderList* renderList)
		{
			if (m_init)
			{
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<GPUCommandBufferVulkan*>(cmdBuffer)->GetCmdBuffer());
			}
		});

		imguiPass.OnEndRender([this](Graphics::RenderPass* renderPass)
		{
			m_init = true;
		});
#endif
	}

	void VulkanImGUIRenderer::Init()
	{
#if defined(IMGUI_ENABLED)
		GPUDeviceVulkan& device = static_cast<GPUDeviceVulkan&>(*Graphics::GPUDevice::Instance());
		GPUAdapterVulkan& adapter = static_cast<GPUAdapterVulkan&>(*device.GetAdapter());

		SetupImGuiRenderPass();
		SetupImGuiDescriptorPool();

		ImGui_ImplGlfw_InitForVulkan(Window::m_window, false);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = device.m_instance;
		init_info.PhysicalDevice = adapter.Gpu;
		init_info.Device = device.Device;
		init_info.QueueFamily = device.GetQueueFamilyIndex(GPUQueue::GRAPHICS);
		init_info.Queue = device.GetQueue(GPUQueue::GRAPHICS);
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = m_descriptorPool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		init_info.ImageCount = 3;
		ImGui_ImplVulkan_Init(&init_info, m_renderpass);

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

		GPUCommandBufferVulkan* command_buffer = static_cast<GPUCommandBufferVulkan*>(GPUCommandBufferVulkan::New());
		command_buffer->Init(Graphics::GPUCommandBufferDesc::CreateOneTimeCmdBuffer());
		command_buffer->BeginRecord();
		ImGui_ImplVulkan_CreateFontsTexture(command_buffer->GetCmdBuffer());
		command_buffer->EndRecord();
		command_buffer->SubmitAndWait(GPUQueue::GRAPHICS);
		::Delete(command_buffer);
#endif
	}

	void VulkanImGUIRenderer::SetupImGuiRenderPass()
	{
		//auto device = VulkanDevice::Instance();

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

	void VulkanImGUIRenderer::SetupImGuiDescriptorPool()
	{
		//1: create descriptor pool for IMGUI
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = (u32)std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		GPUDeviceVulkan& device = static_cast<GPUDeviceVulkan&>(*Graphics::GPUDevice::Instance());
		ThrowIfFailed(vkCreateDescriptorPool(device.Device, &pool_info, nullptr, &m_descriptorPool));
	}

	void VulkanImGUIRenderer::WindowResize(Event const& event)
	{
#if defined(IMGUI_ENABLED)
		//VulkanResizeEvent resizeEvent = static_cast<const VulkanResizeEvent&>(event);

		//ImGuiIO& io = ImGui::GetIO();
		//io.DisplaySize = ImVec2((float)resizeEvent.m_width, (float)resizeEvent.m_height);
		//ImGui_ImplVulkan_SetMinImageCount(2);
#endif
	}
}