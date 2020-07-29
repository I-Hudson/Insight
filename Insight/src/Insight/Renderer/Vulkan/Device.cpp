#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/Device.h"
#include "Insight/Renderer/Vulkan/Queue.h"

#include "Insight/Module/WindowModule.h"
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


		Device::Device(DeviceSettings& deviceSettings)
			: m_deviceSettings(deviceSettings)
		{
			VkInstanceCreateInfo createInfo = VulkanInits::InstanceCreateInfo(VulkanInits::ApplicationInfo(CONFIG_VAL(Config::WindowConfig.WindowTitle).c_str()),
				m_deviceSettings.RequiredExtensions, m_deviceSettings.ValidationLayers, &VulkanInits::DebugCreateInfoEXT(DebugCallback));
			ThrowIfFailed(vkCreateInstance(&createInfo, nullptr, &m_instance));

			ThrowIfFailed(CreateDebugUtilsMessengerEXT(m_instance, &VulkanInits::DebugCreateInfoEXT(DebugCallback), nullptr, &m_debugMessenger));

			CreateSurface();
			SelectPhysicalDevice();
			CreateDevice();
		}

		Device::~Device()
		{
			vkDestroyDevice(m_device, nullptr);
			vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

			DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
			vkDestroyInstance(m_instance, nullptr);
		}

		const QueueFamily& Device::GetQueueFamily(const QueueFamilyType type) const
		{
			for (auto& queue : m_queueFamily)
			{
				if (queue.GetType() == type)
				{
					return queue;
				}
			}
		}

		Queue& Device::GetQueue(const QueueFamilyType type)
		{
			for (auto it = m_queues.begin(); it != m_queues.end(); ++it)
			{
				if ((*it).GetType() == type)
				{
					return (*it);
				}
			}
		}

		uint32_t Device::GetMemoryType(const uint32_t& memoryType, const VkMemoryPropertyFlags& memProb)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((memoryType & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & memProb) == memProb) 
				{
					return i;
				}
			}

		}

		void Device::WaitForIdle()
		{
			vkDeviceWaitIdle(m_device);
		}

		void Device::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				func(instance, m_debugMessenger, pAllocator);
			}
		}

		bool Device::IsDeviceSuitable(VkPhysicalDevice device)
		{
			std::vector<QueueFamily> families
			{
				QueueFamily(QueueFamilyType::Graphics, device, GetSurface()),
				QueueFamily(QueueFamilyType::Present, device, GetSurface())
			};

			//bool extensionsSupported = CheckDeviceExtensionSupport(device);
			//bool swapChainAdequate = false;
			//if (extensionsSupported)
			//{
			//	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			//	swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			//}

			VkPhysicalDeviceProperties pros;
			vkGetPhysicalDeviceProperties(device, &pros);

			bool familySupport = true;
			for (auto& family : families)
			{
				if (!family.IsComplete())
				{
					familySupport = false;
					break;
				}
			}
			return familySupport && pros.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU; //&& extensionsSupported && swapChainAdequate;
		}

		void Device::CreateSurface()
		{
			ThrowIfFailed(glfwCreateWindowSurface(m_instance, m_deviceSettings.Window->m_window, nullptr, &m_surface));
		}

		void Device::SelectPhysicalDevice()
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

			if (deviceCount == 0)
			{
				IS_CORE_ASSERT(true, "Failed to find GPUs with Vulkan support!");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

			for (const auto& device : devices)
			{
				if (IsDeviceSuitable(device))
				{
					m_physicalDevice = device;

					std::vector<QueueFamily> families
					{
						QueueFamily(QueueFamilyType::Graphics, device, GetSurface()),
						QueueFamily(QueueFamilyType::Present, device, GetSurface())
					};
					m_queueFamily = families;
					break;
				}
			}

			if (m_physicalDevice == VK_NULL_HANDLE)
			{
				IS_CORE_ASSERT(false, "Failed to find sutible GPU.");
			}

			IS_CORE_INFO("Found sutible GPU.");
		}

		void Device::CreateDevice()
		{
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies;
			for (auto& family : m_queueFamily)
			{
				uniqueQueueFamilies.insert(family.GetValue());
			}

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
			createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceSettings.DeviceExtensions.size());
			createInfo.ppEnabledExtensionNames = m_deviceSettings.DeviceExtensions.data();

			if (m_deviceSettings.ValidationLayers.size() > 0)
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(m_deviceSettings.ValidationLayers.size());
				createInfo.ppEnabledLayerNames = m_deviceSettings.ValidationLayers.data();
			}
			else
			{
				createInfo.enabledLayerCount = 0;
			}

			ThrowIfFailed(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));
			IS_CORE_INFO("Vulkan logical device completed.");

			m_queues.emplace_back(this, GetQueueFamily(QueueFamilyType::Graphics));
			m_queues.emplace_back(this, GetQueueFamily(QueueFamilyType::Present));

			vkGetDeviceQueue(GetDevice(), GetQueueFamily(QueueFamilyType::Graphics).GetValue(), 0, &m_queues[0].GetQueue());
			vkGetDeviceQueue(GetDevice(), GetQueueFamily(QueueFamilyType::Present).GetValue(), 0, &m_queues[1].GetQueue());
		}
	}
}
#endif