#include "ispch.h"

#include "Insight/Renderer/ImGuiRenderer.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Module/ModuleManager.h"
#include "Insight/Module/WindowModule.h"
#include "Insight/Module/GraphicsModule.h"
#include "Insight/Library/Library.h"
#include "Insight/Event/EventManager.h"
#include "Insight/Instrumentor/Instrumentor.h"

#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/VulkanRenderer.h"
#include "Insight/Renderer/Vulkan/VulkanFramebuffer.h"
#include "Insight/Renderer/Vulkan/VulkanMaterial.h"
#include "Insight/Renderer/Vulkan/VulkanShader.h"
#include "Insight/Renderer/Vulkan/CommandPool.h"
#include "Insight/Renderer/Vulkan/CommandBuffer.h"
#elif 

#endif

namespace Insight
{
	namespace Render
	{
		ImGuiRenderer::ImGuiRenderer(Renderer* renderer)
		{
			SetInstancePtr(this);

			EventManager::Bind(EventType::WindowResize, typeid(ImGuiRenderer).name(), BIND_FUNC(ImGuiRenderer::WindowResize, this));

			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Docking
			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Viewport

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsClassic();

#ifdef IS_VULKAN
			m_vulkanRenderer = dynamic_cast<VulkanRenderer*>(renderer);
			const Window* window = Module::WindowModule::GetWindow();

			m_framebuffer = NEW_ON_HEAP(VulkanFramebuffer, m_vulkanRenderer->GetDeviceWrapper(), window->GetWidth(), window->GetHeight());
			m_framebuffer->CreateAttachment(VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_framebuffer->CompileFrameBuffer();

			m_commandPool = NEW_ON_HEAP(CommandPool, m_vulkanRenderer->GetDeviceWrapper(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			m_commandBuffer = m_commandPool->AllocCommandBuffer();

            CreateDescPool();

			// Setup Platform/Renderer bindings
			ImGui_ImplGlfw_InitForVulkan(Module::WindowModule::GetWindow()->m_window, true);
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = m_vulkanRenderer->GetInstance();
			init_info.PhysicalDevice = m_vulkanRenderer->GetPhysicalDevice();
			init_info.Device = m_vulkanRenderer->GetDevice();
			init_info.QueueFamily = m_vulkanRenderer->GetGraphicsQueue().GetQueueFamily().GetValue();
			init_info.Queue = m_vulkanRenderer->GetGraphicsQueue().GetQueue();
			init_info.PipelineCache = nullptr;
            init_info.DescriptorPool = m_descPool;
			init_info.Allocator = nullptr;
			init_info.MinImageCount = 2;
			init_info.ImageCount = 3;
			init_info.CheckVkResultFn = [](VkResult err)
			{
				if (err == 0)
				{
					return;
				}
				fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
				if (err < 0)
				{
					abort();
				}
			};
			ImGui_ImplVulkan_Init(&init_info, m_framebuffer->GetRenderpass()->GetRenderpass());

			m_commandBuffer->StartRecord();

			ImGui_ImplVulkan_CreateFontsTexture(m_commandBuffer->GetBuffer());
            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &m_commandBuffer->GetBuffer();
			m_commandBuffer->EndRecord();

            ThrowIfFailed(vkQueueSubmit(m_vulkanRenderer->GetGraphicsQueue().GetQueue(), 1, &end_info, VK_NULL_HANDLE));

            ThrowIfFailed(vkDeviceWaitIdle(m_vulkanRenderer->GetDevice()));

			ImGui_ImplVulkan_DestroyFontUploadObjects();

#elif defined(IS_OPENGL)

#endif // IS_VULKAN

		}

		ImGuiRenderer::~ImGuiRenderer()
		{
#ifdef IS_VULKAN
			EventManager::Unbind(EventType::WindowResize, typeid(ImGuiRenderer).name());
			DELETE_ON_HEAP(m_framebuffer);
			DELETE_ON_HEAP(m_commandPool);

			vkDestroyDescriptorPool(m_vulkanRenderer->GetDevice(), m_descPool, nullptr);

			ImGui_ImplVulkan_Shutdown();
#endif
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

			ClearPtr();
		}

		void ImGuiRenderer::NewFrame()
		{
#ifdef IS_VULKAN
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
#elif defined(IS_OPENGL)

#endif // IS_VULKAN
			ImGui::NewFrame();

#ifdef IS_EDITOR
			// Place all of this into a layer stack system.
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->GetWorkPos());
			ImGui::SetNextWindowSize(viewport->GetWorkSize());
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", 0, window_flags);
			ImGui::PopStyleVar();

			ImGui::PopStyleVar(2);

			ImGuiIO& io = ImGui::GetIO();
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

			ImGui::End();
#endif
		}

		void ImGuiRenderer::EndFrame()
		{
			//ImGuiIO& io = ImGui::GetIO();
			//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			//{
			//	GLFWwindow* backup_current_context = glfwGetCurrentContext();
			//	ImGui::UpdatePlatformWindows();
			//	ImGui::RenderPlatformWindowsDefault();
			//	glfwMakeContextCurrent(backup_current_context);
			//}
		}

		void ImGuiRenderer::Render(CommandBuffer* commandBuffer)
		{
			IS_PROFILE_FUNCTION();

			ImGui::EndFrame();

			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2((float)Module::WindowModule::GetWindow()->GetWidth(), (float)Module::WindowModule::GetWindow()->GetHeight());

			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();
#ifdef IS_VULKAN
			// Record dear imgui primitives into command buffer
			ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer->GetBuffer());
#endif
		}

		void ImGuiRenderer::WindowResize(const Event& event)
		{
			const WindowResizeEvent e = static_cast<const WindowResizeEvent&>(event);
			m_framebuffer->Resize(e.m_width, e.m_height);
		}

#ifdef IS_VULKAN

        void ImGuiRenderer::CreateDescPool()
        {
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
            pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            ThrowIfFailed(vkCreateDescriptorPool(m_vulkanRenderer->GetDevice(), &pool_info, nullptr, &m_descPool));
        }
#endif
	}
}