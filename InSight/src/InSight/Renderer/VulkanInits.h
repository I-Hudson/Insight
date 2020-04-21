#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Insight
{
	namespace Render
	{
		class VulkanInits
		{
		public:

			static VkApplicationInfo ApplicationInfo(const char* appTitle, const char* engineTitle = "Engine")
			{
				VkApplicationInfo appInfo = {};
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pApplicationName = appTitle;
				appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				appInfo.pEngineName = engineTitle;
				appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
				appInfo.apiVersion = VK_API_VERSION_1_0;

				return appInfo;
			}

			static VkInstanceCreateInfo InstanceCreateInfo(const VkApplicationInfo& appInfo, const std::vector<const char*>& extensions, 
															const std::vector<const char*>& validationLayers = std::vector<const char*>(),
															void* debugCreateInfo = nullptr)
			{
				VkInstanceCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				createInfo.pApplicationInfo = &appInfo;
				createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
				createInfo.ppEnabledExtensionNames = extensions.data();
				createInfo.enabledLayerCount = 0;

				if (validationLayers.size() > 0)
				{
					createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
					createInfo.ppEnabledLayerNames = validationLayers.data();

					createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)debugCreateInfo;
				}
				else
				{
					createInfo.enabledLayerCount = 0;
					createInfo.pNext = nullptr;
				}

				return createInfo;
			}

			static VkDeviceCreateInfo DeviceInfo()
			{
				VkDeviceCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

				return createInfo;
			}

			static VkDeviceQueueCreateInfo DeviceQueueInfo()
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

				return queueCreateInfo;
			}

			static VkSwapchainCreateInfoKHR SwapChainInfo()
			{
				VkSwapchainCreateInfoKHR createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				return createInfo;
			}

			static VkImageViewCreateInfo ImageViewInfo()
			{
				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				return createInfo;
			}

			static VkDescriptorPoolSize DescPoolSize(const int& count)
			{
				VkDescriptorPoolSize poolSize = {};
				poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSize.descriptorCount = count;

				return poolSize;
			}

			static VkDescriptorPoolCreateInfo DescPoolCreateInfo(const int& count, VkDescriptorPoolSize poolSize, const int& maxSets)
			{
				VkDescriptorPoolCreateInfo poolInfo = {};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = count;
				poolInfo.pPoolSizes = &poolSize;
				poolInfo.maxSets = maxSets;

				return poolInfo;
			}

			static VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfoEXT(PFN_vkDebugUtilsMessengerCallbackEXT callback)
			{
				VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
				createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
				createInfo.pfnUserCallback = callback;
				createInfo.pUserData = nullptr; // Optional

				return createInfo;
			}
		};
	}
}