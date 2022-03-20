#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUPipelineStateObject_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUSwapchain_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUBuffer_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUFence_Vulkan.h"
#include "Graphics/Window.h"
#include "Core/Defines.h"
#include "Core/Logger.h"

#include <iostream>

#ifdef IS_PLATFORM_WIN32
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#define VMA_IMPLEMENTATION
#include "VmaUsage.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			static constexpr const char* g_DeviceExtensions[] =
			{
				#if VK_KHR_maintenance1 && !VK_VERSION_1_1
				VK_KHR_MAINTENANCE1_EXTENSION_NAME,
				#endif

				#if VK_KHR_swapchain
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,
				#endif

				#if VK_EXT_validation_cache
				VK_EXT_VALIDATION_CACHE_EXTENSION_NAME,
				#endif

				#if VK_KHR_sampler_mirror_clamp_to_edge && !VK_VERSION_1_2
				VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
				#endif

				#if VK_KHR_maintenance3 && !VK_VERSION_1_1
				VK_KHR_MAINTENANCE3_EXTENSION_NAME,
				#endif

				#if VK_EXT_descriptor_indexing && !VK_VERSION_1_2
				VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
				#endif
			};

			std::vector<const char*> StringVectorToConstChar(const std::vector<std::string>& vec)
			{
				std::vector<const char*> v;
				for (auto& value : vec)
				{
					v.push_back(value.c_str());
				}
				return v;
			}

            struct LayerExtension
            {
                VkLayerProperties Layer;
                std::vector<VkExtensionProperties> Extensions;

                LayerExtension()
                {
                    //Platform::MemClear(&Layer, sizeof(Layer));
                }

                void GetExtensions(std::vector<std::string>& result)
                {
                    for (auto& e : Extensions)
                    {
                        result.push_back(e.extensionName);
                    }
                }

                void GetExtensions(std::vector<const char*>& result)
                {
                    for (auto& e : Extensions)
                    {
                        result.push_back(e.extensionName);
                    }
                }
            };

            void ThrowIfFailed(VkResult res)
            { }

			PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
			PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
			VkDebugUtilsMessengerEXT debugUtilsMessenger;
			VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData)
			{
				if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT || 
					messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ||
					messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ||
					messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ||
					messageSeverity & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
				{
					IS_CORE_ERROR("Id: {}\n Name: {}\n Msg: {}", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
				}

				// The return value of this callback controls whether the Vulkan call that caused the validation message will be aborted or not
				// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message to abort
				// If you instead want to have calls abort, pass in VK_TRUE and the function will return VK_ERROR_VALIDATION_FAILED_EXT 
				return VK_FALSE;
			}

			GPUDevice_Vulkan::~GPUDevice_Vulkan()
			{
			}

			bool GPUDevice_Vulkan::Init()
			{
				if (m_instnace && m_device)
				{
					//ERROR MESSAGE
					return true;
				}

				vk::ApplicationInfo applicationInfo = vk::ApplicationInfo(
					"ApplciationName",
					0,
					"Insight",
					0,
					VK_API_VERSION_1_2);

				std::vector<const char*> enabledLayerNames =
				{
					"VK_LAYER_KHRONOS_validation",
				};
				std::vector<const char*> enabledExtensionNames =
				{
#if VK_KHR_surface
					VK_KHR_SURFACE_EXTENSION_NAME,
#endif

#ifdef IS_PLATFORM_WIN32
#if VK_KHR_win32_surface
					VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#endif

#if VK_KHR_display
					VK_KHR_DISPLAY_EXTENSION_NAME,
#endif

#if VK_KHR_get_display_properties2
					VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME,
#endif        

#if VK_EXT_debug_utils
					VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif

#if VK_KHR_get_physical_device_properties2 && !VK_VERSION_1_1
					VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#endif
			};

				vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo(
					{ }, 
					&applicationInfo, 
					enabledLayerNames, 
					enabledExtensionNames);
				m_instnace = vk::createInstance(instanceCreateInfo);

				vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instnace, "vkCreateDebugUtilsMessengerEXT"));
				vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instnace, "vkDestroyDebugUtilsMessengerEXT"));

				VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
				debugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
				debugUtilsMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
				debugUtilsMessengerCI.pfnUserCallback = DebugUtilsMessengerCallback;
				VkResult result = vkCreateDebugUtilsMessengerEXT(m_instnace, &debugUtilsMessengerCI, nullptr, &debugUtilsMessenger);

				m_adapter = FindAdapter();

				std::vector<QueueInfo> queueInfo = {};
				std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos = GetDeviceQueueCreateInfos(queueInfo);
				int queueSize = 0;
				for (size_t i = 0; i < deviceQueueCreateInfos.size(); ++i)
				{
					vk::DeviceQueueCreateInfo& createInfo = deviceQueueCreateInfos[i];
					queueSize += createInfo.queueCount;
				}

				std::vector<float> queuePriorities;
				queuePriorities.reserve(queueSize);
				queueSize = 0;
				for (size_t i = 0; i < deviceQueueCreateInfos.size(); ++i)
				{
					vk::DeviceQueueCreateInfo& createInfo = deviceQueueCreateInfos[i];
					createInfo.pQueuePriorities = queuePriorities.data() + queueSize;
					for (size_t j = 0; j < createInfo.queueCount; ++j)
					{
						queuePriorities.push_back(1.0f);
						++queueSize;
					}
				}

				std::vector<std::string> deviceLayers;
				std::vector<std::string> deviceExtensions;
				GetDeviceExtensionAndLayers(deviceExtensions, deviceLayers);

				std::vector<const char*> deviceLayersCC = StringVectorToConstChar(deviceLayers);
				std::vector<const char*> deviceExtensionsCC = StringVectorToConstChar(deviceExtensions);

				const vk::PhysicalDeviceFeatures deviceFeatures = m_adapter.GetPhysicalDevice().getFeatures();
				
				vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo({}, deviceQueueCreateInfos, deviceLayersCC, deviceExtensionsCC, &deviceFeatures);
				m_device = m_adapter.GetPhysicalDevice().createDevice(deviceCreateInfo);

				for (size_t i = 0; i < queueInfo.size(); ++i)
				{
					const QueueInfo& info = queueInfo[i];
					m_queues[info.Queue] = m_device.getQueue(info.FamilyQueueIndex, info.FamilyQueueIndex);
					m_queueFamilyLookup[info.Queue] = info.FamilyQueueIndex;
				}

				// Initialise vulkan memory allocator
				VmaAllocatorCreateInfo vmaAllocatorInfo{};
				vmaAllocatorInfo.instance = m_instnace;
				vmaAllocatorInfo.physicalDevice = m_adapter.GetPhysicalDevice();
				vmaAllocatorInfo.device = m_device;
				vmaCreateAllocator(&vmaAllocatorInfo, &m_vmaAllocator);

				m_swapchain = new GPUSwapchain_Vulkan();
				m_swapchain->Prepare();
				m_swapchain->Build(GPUSwapchainDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight()));

				GPUCommandListManager::Instance().Create();

				return true;
			}

			void GPUDevice_Vulkan::Destroy()
			{
				WaitForGPU();

				if (m_swapchain)
				{
					m_swapchain->Destroy();
					delete m_swapchain;
					m_swapchain = nullptr;
				}

				GPUBufferManager::Instance().Destroy();
				GPURenderpassManager_Vulkan::Instance().Destroy();
				GPUPipelineStateObjectManager::Instance().Destroy();
				GPUShaderManager::Instance().Destroy();
				GPUCommandListManager::Instance().Destroy();
				GPUSemaphoreManager::Instance().Destroy();
				GPUFenceManager::Instance().Destroy();

				vmaDestroyAllocator(m_vmaAllocator);

				m_device.destroy();
				m_device = nullptr;

				if (debugUtilsMessenger)
				{
					vkDestroyDebugUtilsMessengerEXT(m_instnace, debugUtilsMessenger, nullptr);
				}

				m_instnace.destroy();
				m_instnace = nullptr;
			}

			void GPUDevice_Vulkan::WaitForGPU() const
			{
				m_device.waitIdle();
			}

			GPUAdapter_Vulkan* GPUDevice_Vulkan::GetAdapter()
			{
				return &m_adapter;
			}

			vk::Queue& GPUDevice_Vulkan::GetQueue(GPUQueue queue)
			{
				switch (queue)
				{
				case Insight::Graphics::GPUQueue_Graphics:
				case Insight::Graphics::GPUQueue_Compute:
				case Insight::Graphics::GPUQueue_Transfer:
					return m_queues[queue];
				default: break;
				}
				return m_queues[GPUQueue_Graphics];
			}

			GPUAdapter_Vulkan GPUDevice_Vulkan::FindAdapter()
			{
				std::vector<vk::PhysicalDevice> physicalDevices = m_instnace.enumeratePhysicalDevices();
				GPUAdapter_Vulkan adapter(nullptr);
				for (auto& gpu : physicalDevices)
				{
					adapter = GPUAdapter_Vulkan(gpu);
					if (adapter.GetPhysicalDeviceProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu ||
						(adapter.IsNVIDIA() || adapter.IsAMD()))
					{
						break;
					}
				}
				return adapter;
			}

			std::vector<vk::DeviceQueueCreateInfo> GPUDevice_Vulkan::GetDeviceQueueCreateInfos(std::vector<QueueInfo>& queueInfo)
			{
				std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = {};
				std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_adapter.GetPhysicalDevice().getQueueFamilyProperties();
				int graphicsQueue = -1;
				int computeQueue = -1;
				int transferQueue = -1;
				for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
				{
					const vk::QueueFamilyProperties& queueProp = queueFamilyProperties[i];
					vk::DeviceQueueCreateInfo createInfo = vk::DeviceQueueCreateInfo({}, static_cast<u32>(i), queueProp.queueCount);
					if ((queueProp.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
					{
						queueInfo.push_back(QueueInfo{ static_cast<int>(i), GPUQueue::GPUQueue_Graphics });
						graphicsQueue = static_cast<int>(i);
					}
					if ((queueProp.queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute && computeQueue == -1 && graphicsQueue != i)
					{
						queueInfo.push_back(QueueInfo{ static_cast<int>(i), GPUQueue::GPUQueue_Compute });
						computeQueue = static_cast<int>(i);
					}
					if ((queueProp.queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer && 
						transferQueue == -1 && (queueProp.queueFlags & vk::QueueFlagBits::eGraphics) != vk::QueueFlagBits::eGraphics && 
						(queueProp.queueFlags & vk::QueueFlagBits::eCompute) != vk::QueueFlagBits::eCompute)
					{
						queueInfo.push_back(QueueInfo{ static_cast<int>(i), GPUQueue::GPUQueue_Transfer });
						transferQueue = static_cast<int>(i);
					}
					queueCreateInfos.push_back(createInfo);
				}

				return queueCreateInfos;
			}

			void GPUDevice_Vulkan::GetDeviceExtensionAndLayers(std::vector<std::string>& extensions, std::vector<std::string>& layers)
			{
                std::vector<vk::LayerProperties> layerProperties = m_adapter.GetPhysicalDevice().enumerateDeviceLayerProperties();
                std::vector<vk::ExtensionProperties> extensionProperties = m_adapter.GetPhysicalDevice().enumerateDeviceExtensionProperties();

                const char* vkLayerKhronosValidation = "VK_LAYER_KHRONOS_validation";
                bool hasKhronosStandardValidationLayer = std::find_if(layerProperties.begin(), layerProperties.end(), [vkLayerKhronosValidation](const vk::LayerProperties& layer)
                    {
						return strcmp(layer.layerName, vkLayerKhronosValidation);
                    }) != layerProperties.end();
                if (hasKhronosStandardValidationLayer)
                {
                    if (true/*(bool)CONFIG_VAL(Config::GraphicsConfig.Validation)*/)
                    {
                        layers.push_back(vkLayerKhronosValidation);
                    }
                }

				IS_CORE_INFO("Device layers:");
				for (size_t i = 0; i < layerProperties.size(); ++i)
				{
					IS_CORE_INFO("{}", layerProperties[i].layerName);
				}
				std::cout << "Device extensions:\n";
				for (size_t i = 0; i < extensionProperties.size(); ++i)
				{
					IS_CORE_INFO("{}", extensionProperties[i].extensionName);
				}

				for (size_t i = 0; i < ARRAY_COUNT(g_DeviceExtensions); i++)
				{
					const char* ext = g_DeviceExtensions[i];
					if (std::find_if(extensionProperties.begin(), extensionProperties.end(), [ext](const vk::ExtensionProperties& extnesion)
					{
						return strcmp(extnesion.extensionName, ext);
					}) != extensionProperties.end())
					{
						extensions.push_back(ext);
					}
				}
			}
		}
	}
}