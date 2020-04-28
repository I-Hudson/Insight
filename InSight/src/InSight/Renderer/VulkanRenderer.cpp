#include "ispch.h"
#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/VulkanRenderer.h"
#include "Insight/Renderer/VulkanInits.h"
#include "Insight/Renderer/Lowlevel/QueueFamily.h"

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
			Memory::MemoryManager::DeleteOnFreeList(m_swapchain);
			Memory::MemoryManager::DeleteOnFreeList(m_device);
		}

		void VulkanRenderer::Clear()
		{
		}

		void VulkanRenderer::Render()
		{
			m_swapchain->Draw();
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
