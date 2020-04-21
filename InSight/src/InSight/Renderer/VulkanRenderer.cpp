#include "ispch.h"
#include "Insight/Renderer/VulkanRenderer.h"
#include "Insight/Renderer/VulkanInits.h"
#include "Insight/Log.h"
#include "Insight/Renderer/Vulkan.h"

#include "Insight/Config/Config.h"

namespace Insight
{
	namespace Render
	{
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback
		(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			{
				IS_CORE_INFO("Validation layer : {0}", pCallbackData->pMessage);
			}
			else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				IS_CORE_WARN("Validation layer : {0}", pCallbackData->pMessage);
			}
			else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			{
				IS_CORE_ERROR("Validation layer : {0}", pCallbackData->pMessage);
			}
			else
			{
				IS_CORE_TRACE("Validation layer : {0}", pCallbackData->pMessage);
			}

			return VK_FALSE;
		}

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

			std::vector<const char*> extensionsNames = GetRequiredExtensions();
			VkInstanceCreateInfo createInfo = VulkanInits::InstanceCreateInfo(VulkanInits::ApplicationInfo(CONFIG_VAL(Config::WindowConfig.WindowTitle).c_str()), 
				extensionsNames, m_validationLayers, &VulkanInits::DebugCreateInfoEXT(DebugCallback));
			ThrowIfFailed(vkCreateInstance(&createInfo, nullptr, &m_inst));
			SetupDebugMessenger();
			CreateSurface();
			SelectPhysicalDevice();
			CreateLogicalDevice();
			CreateSwapChain();
			CreateSwapChainImageViews();

			IS_CORE_INFO("Vulkan Setup Complete.");
		}

		VulkanRenderer::~VulkanRenderer()
		{
			for (auto swapChainImage : m_swapChainImages)
			{
				vkDestroyImageView(m_device, swapChainImage.View, nullptr);
			}

			vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
			vkDestroyDevice(m_device, nullptr);

			DestroyDebugUtilsMessengerEXT(m_inst, m_debugMessenger, nullptr);
			vkDestroySurfaceKHR(m_inst, m_surface, nullptr);
			vkDestroyInstance(m_inst, nullptr);
		}

		void VulkanRenderer::Clear()
		{
		}

		void VulkanRenderer::Render()
		{
		}

		void VulkanRenderer::Present()
		{
		}

		void VulkanRenderer::CreateSurface()
		{
			ThrowIfFailed(glfwCreateWindowSurface(m_inst, m_windowModule->GetWindow()->m_window, nullptr, &m_surface));
		}

		void VulkanRenderer::SelectPhysicalDevice()
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(m_inst, &deviceCount, nullptr);

			if (deviceCount == 0)
			{
				throw std::runtime_error("failed to find GPUs with Vulkan support!");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(m_inst, &deviceCount, devices.data());

			for (const auto& device : devices)
			{
				if (IsDeviceSuitable(device)) 
				{
					m_physicalDevice = device;
					break;
				}
			}

			if (m_physicalDevice == VK_NULL_HANDLE)
			{
				IS_CORE_ASSERT("Failed to find sutible GPU.");
			}

			IS_CORE_INFO("Found sutible GPU.");
		}

		void VulkanRenderer::CreateLogicalDevice()
		{
			QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			float queuePriority = 1.0f;
			for (uint32_t queueFamily : uniqueQueueFamilies)
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			VkPhysicalDeviceFeatures deviceFeatures = {};
			vkGetPhysicalDeviceFeatures(m_physicalDevice, &deviceFeatures);

			VkDeviceCreateInfo createInfo = VulkanInits::DeviceInfo();
			createInfo.pQueueCreateInfos = queueCreateInfos.data();
			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pEnabledFeatures = &deviceFeatures;
			createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

			if (m_enableValidationLayers)
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
				createInfo.ppEnabledLayerNames = m_validationLayers.data();
			}
			else
			{
				createInfo.enabledLayerCount = 0;
			}

			ThrowIfFailed(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));
			IS_CORE_INFO("Vulkan logical device completed.");

			vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
			vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
		}

		bool VulkanRenderer::IsDeviceSuitable(VkPhysicalDevice device)
		{
			QueueFamilyIndices indices = FindQueueFamilies(device);

			bool extensionsSupported = CheckDeviceExtensionSupport(device);
			bool swapChainAdequate = false;
			if (extensionsSupported) 
			{
				SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			}

			return indices.graphicsFamily.has_value() && extensionsSupported && swapChainAdequate;
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

		QueueFamilyIndices VulkanRenderer::FindQueueFamilies(VkPhysicalDevice device)
		{
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			int i = 0;
			for (const auto& queueFamily : queueFamilies)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					indices.graphicsFamily = i;
				}

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

				if (presentSupport)
				{
					indices.presentFamily = i;
				}

				if (indices.IsComplete())
				{
					break;
				}

				i++;
			}

			return indices;
		}

		SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(VkPhysicalDevice device)
		{
			SwapChainSupportDetails details;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
			if (formatCount != 0) 
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
			if (presentModeCount != 0) 
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		VkSurfaceFormatKHR VulkanRenderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
		{
			for (const auto& availableFormat : formats) 
			{
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return availableFormat;
				}
			}

			return formats[0];
		}

		VkPresentModeKHR VulkanRenderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
		{
			for (const auto& availablePresentMode : presentModes)
			{
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return availablePresentMode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D VulkanRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
		{
			if (capabilities.currentExtent.width != UINT32_MAX)
			{
				return capabilities.currentExtent;
			}
			else 
			{
				uint32_t width = static_cast<uint32_t>(m_windowModule->GetWindow()->GetWidth());
				uint32_t height = static_cast<uint32_t>(m_windowModule->GetWindow()->GetHeight());
				VkExtent2D actualExtent = { width, height };

				actualExtent.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}

		void VulkanRenderer::CreateSwapChain()
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

			VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
			{
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo = VulkanInits::SwapChainInfo();
			createInfo.surface = m_surface;
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
			uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			if (indices.graphicsFamily != indices.presentFamily) 
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 0; // Optional
				createInfo.pQueueFamilyIndices = nullptr; // Optional
			}

			createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			ThrowIfFailed(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain));

			imageCount = 0;
			std::vector<VkImage> swapChainImages;
			vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, swapChainImages.data());

			IS_CORE_INFO("SwapChain completed.");

			for (size_t i = 0; i < imageCount; i++)
			{
				VulkanImage image
				{
					swapChainImages[i],
					nullptr,
					nullptr,
					surfaceFormat.format,
					extent
				};

				m_swapChainImages.push_back(image);
			}
		}

		void VulkanRenderer::CreateSwapChainImageViews()
		{
			for (size_t i = 0; i < m_swapChainImages.size(); i++)
			{
				VkImageViewCreateInfo createInfo = VulkanInits::ImageViewInfo();
				createInfo.image = m_swapChainImages[i].Image;
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = m_swapChainImages[i].Format;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				ThrowIfFailed(vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImages[i].View));
			}

			IS_CORE_INFO("SwapChain image view completed.");
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

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr) {
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else {
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		void VulkanRenderer::SetupDebugMessenger()
		{
			ThrowIfFailed(CreateDebugUtilsMessengerEXT(m_inst, &VulkanInits::DebugCreateInfoEXT(DebugCallback), nullptr, &m_debugMessenger));
		}

		void VulkanRenderer::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				func(instance, m_debugMessenger, pAllocator);
			}
		}
	}
}
