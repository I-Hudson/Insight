#include "ispch.h"
#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/VulkanRenderer.h"
#include "Insight/Renderer/VulkanInits.h"
#include "Insight/Renderer/Lowlevel/QueueFamily.h"
#include "Insight/Module/WindowModule.h"

#include "Insight/Log.h"

#include "Insight/Memory/MemoryManager.h"

#include "Insight/Config/Config.h"

namespace Insight
{
	namespace Render
	{
		VulkanRenderer::VulkanRenderer(RendererStartUpData& startupData)
		{
			m_windowModule = startupData.WindowModule;

			m_validationLayers =
			{
				"VK_LAYER_KHRONOS_validation"
			};

			m_deviceExtensions =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			if (!CheckValidationLayerSupport(m_validationLayers))
			{
				m_validationLayers.clear();
			}

			DeviceSettings deviceSettings
			{
				m_validationLayers,
				m_deviceExtensions,
				GetRequiredExtensions()
			};
			m_device = Memory::MemoryManager::NewOnFreeList<Device>(deviceSettings);

			m_framebuffer = Memory::MemoryManager::NewOnFreeList<Framebuffer>(m_device, m_windowModule->GetWindow()->GetWidth(), m_windowModule->GetWindow()->GetHeight());
			m_framebuffer->CreateAttachment(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
											VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_framebuffer->CreateAttachment(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
											VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_framebuffer->CompileFrameBuffer();

			std::vector<ShaderModuleBase> shaderModules =
			{ VertexShader(m_device, "shader - Copy.vert"),
			  FragmentShader(m_device, "shader - Copy.frag")
			};
			ShaderData data
			{
				m_device,
				shaderModules,
				VkExtent2D{ static_cast<uint32_t>(m_windowModule->GetWindow()->GetWidth()), static_cast<uint32_t>(m_windowModule->GetWindow()->GetHeight())},
				m_framebuffer->GetRenderpass()
			};
			m_shader = Memory::MemoryManager::NewOnFreeList<Shader>(data);

			m_commandPool = Memory::MemoryManager::NewOnFreeList<CommandPool>(m_device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			m_commandBuffer = m_commandPool->AllocCommandBuffer();

			SwapchainSettings swapchainSettings
			{
				m_windowModule->GetWindow(),
				m_device
			};
			m_swapchain = Memory::MemoryManager::NewOnFreeList<Swapchain>(swapchainSettings);

			IS_CORE_INFO("Vulkan Setup Complete.");
		}

		VulkanRenderer::~VulkanRenderer()
		{
			m_device->WaitForIdle();

			Memory::MemoryManager::DeleteOnFreeList(m_shader);
			Memory::MemoryManager::DeleteOnFreeList(m_framebuffer);
			Memory::MemoryManager::DeleteOnFreeList(m_commandPool);

			Memory::MemoryManager::DeleteOnFreeList(m_swapchain);
			Memory::MemoryManager::DeleteOnFreeList(m_device);
		}

		void VulkanRenderer::Clear()
		{
		}

		void VulkanRenderer::Render()
		{
			m_swapchain->AcquireNextImage();

			m_commandBuffer->StartRecord();
			m_framebuffer->BindBuffer(m_commandBuffer);
			m_shader->Bind(m_commandBuffer);
			vkCmdDraw(m_commandBuffer->GetBuffer(), 3, 1, 0, 0);
			m_framebuffer->UnbindBuffer(m_commandBuffer);
			m_commandBuffer->EndRecord();

			std::vector<VkSemaphore> waitSemaphores = { m_swapchain->GetCurrentFinishedFrame()->GetSemaphore() };
			std::vector<VkPipelineStageFlags> stageFlags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			std::vector<VkCommandBuffer> commandBuffers = { m_commandBuffer->GetBuffer() };
			std::vector<VkSemaphore> signalSemaphore = { m_framebuffer->GetFinishedSem()->GetSemaphore() };
			VkSubmitInfo submitInfo = VulkanInits::SubmitInfo(waitSemaphores, stageFlags, commandBuffers, signalSemaphore);

			GraphicsQueueInfo info;
			info.SubmitInfo = &submitInfo;
			info.SyncFence = VK_NULL_HANDLE;
			m_device->GetQueue(QueueFamilyType::Graphics).Submit(info);

			m_swapchain->Draw(m_framebuffer->GetFinishedSem());
		}

		void VulkanRenderer::Present()
		{
			m_swapchain->Present();
		}

		bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

			for (const auto& extension : availableExtensions) {
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();
		}

		bool VulkanRenderer::CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const char* layerName : validationLayers) {
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers) {
					if (strcmp(layerName, layerProperties.layerName) == 0) {
						layerFound = true;
						break;
					}
				}

				if (!layerFound) {
					return false;
				}
			}

			return true;
		}

		std::vector<const char*> VulkanRenderer::GetRequiredExtensions()
		{
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensionVector(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if (m_enableValidationLayers)
			{
				extensionVector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			return extensionVector;
		}

		void VulkanRenderer::VulkanEnumExtProps()
		{
			UINT numExt = 0;
			ThrowIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &numExt, nullptr));


			printf("Found %d extensions \n", numExt);
			std::vector<VkExtensionProperties> props;
			props.resize(numExt);

			ThrowIfFailed(vkEnumerateInstanceExtensionProperties(NULL, &numExt, &props[0]));

			uint32_t extensionCompleted = 0;
			std::vector<const char*> extensions = GetRequiredExtensions();

			for (UINT i = 0; i < numExt; ++i)
			{
				printf("Instance extension %d - %s\n", i, props[i].extensionName);

				for (size_t j = 0; j < extensions.size(); ++j)
				{
					const char* reqEx = extensions[j];
					if (strcmp(props[i].extensionName, reqEx) == 0)
					{
						extensionCompleted++;
						break;
					}
				}
			}

			IS_CORE_INFO("Required extensions found: {0}", extensionCompleted == extensions.size());
		}
	}
}
