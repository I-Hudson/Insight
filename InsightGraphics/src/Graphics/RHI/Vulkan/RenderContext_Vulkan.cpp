#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/Window.h"
#include "Core/Logger.h"

#ifdef IS_PLATFORM_WIN32
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

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

			bool RenderContext_Vulkan::Init()
			{
				if (m_instnace && m_device)
				{
					IS_CORE_ERROR("[RenderContext_Vulkan::Init] Context already inited.");
					return true;
				}

				m_instnace = CreateInstance();

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

				const vk::PhysicalDeviceFeatures deviceFeatures = m_adapter.getFeatures();

				vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo({}, deviceQueueCreateInfos, deviceLayersCC, deviceExtensionsCC, &deviceFeatures);
				m_device = m_adapter.createDevice(deviceCreateInfo);

				for (size_t i = 0; i < queueInfo.size(); ++i)
				{
					const QueueInfo& info = queueInfo[i];
					m_commandQueues[info.Queue] = m_device.getQueue(info.FamilyQueueIndex, info.FamilyQueueIndex);
					m_queueFamilyLookup[info.Queue] = info.FamilyQueueIndex;
				}

				VkSurfaceKHR surfaceKHR;
				VkResult res = glfwCreateWindowSurface(m_instnace, Window::Instance().GetRawWindow(), nullptr, &surfaceKHR);
				m_surface = vk::SurfaceKHR(surfaceKHR);
				m_swapchain = CreateSwapchain();

				m_pipelineLayoutManager.SetRenderContext(this);
				m_pipelineStateObjectManager.SetRenderContext(this);
				m_renderpassManager.SetRenderContext(this);

				for (FrameResource& frame : m_frames)
				{
					frame.Init(this);
				}

				return true;
			}

			void RenderContext_Vulkan::Destroy()
			{
				m_device.waitIdle();

				for (FrameResource& frame : m_frames)
				{
					frame.Destroy();
				}

				m_vertexBuffers.Destroy();
				m_indexBuffers.Destroy();

				m_shaderManager.Destroy();
				m_pipelineStateObjectManager.Destroy();
				m_pipelineLayoutManager.Destroy();
				m_renderpassManager.Destroy();

				if (m_swapchain)
				{
					for (vk::ImageView& view : m_swapchainImageViews)
					{
						m_device.destroyImageView(view);
					}
					m_swapchainImageViews.clear();
					m_device.destroySwapchainKHR(m_swapchain);
					m_swapchainImages.resize(0);
				}

				if (m_surface)
				{
					m_instnace.destroySurfaceKHR(m_surface);
				}

				m_device.destroy();
				m_device = nullptr;

				if (debugUtilsMessenger)
				{
					vkDestroyDebugUtilsMessengerEXT(m_instnace, debugUtilsMessenger, nullptr);
				}

				m_instnace.destroy();
				m_instnace = nullptr;
			}

			void RenderContext_Vulkan::Render(CommandList cmdList)
			{
				FrameResource& frame = m_frames[m_currentFrame];

				m_device.waitForFences({ frame.SubmitFence }, 1, 0xFFFFFFFF);
				vk::ResultValue nextImage = m_device.acquireNextImageKHR(m_swapchain, 0xFFFFFFFF, frame.SwapchainAcquire);
				m_availableSwapchainImage = nextImage.value;
				m_device.resetFences({ frame.SubmitFence });

				frame.CommandPool.Update();
				CommandList_Vulkan& cmdListVulkan = frame.CommandPool.GetCommandList();
				cmdListVulkan.Record(cmdList);

				std::array<vk::Semaphore, 1> waitSemaphores = { frame.SwapchainAcquire };
				std::array<vk::PipelineStageFlags, 1> dstStageFlgs = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
				std::array<vk::CommandBuffer , 1> commandBuffers = { cmdListVulkan.GetCommandBuffer() };
				std::array<vk::Semaphore, 1> signalSemaphore = { frame.SignalSemaphore };

				vk::SubmitInfo submitInfo = vk::SubmitInfo(
					waitSemaphores,
					dstStageFlgs,
					commandBuffers,
					signalSemaphore);
				m_commandQueues[GPUQueue_Graphics].submit(submitInfo, frame.SubmitFence);

				std::array<vk::Semaphore, 1> signalSemaphores = { frame.SignalSemaphore };
				std::array<vk::SwapchainKHR, 1> swapchains = { m_swapchain };
				std::array<u32, 1> swapchainImageIndex = { m_availableSwapchainImage };

				vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR(signalSemaphores, swapchains, swapchainImageIndex);
				m_commandQueues[GPUQueue_Graphics].presentKHR(presentInfo);
			}

			GPUBuffer* RenderContext_Vulkan::CreateVertexBuffer(u64 sizeBytes)
			{
				GPUBuffer_Vulkan* buffer = m_vertexBuffers.CreateResource();
				return buffer;
			}

			GPUBuffer* RenderContext_Vulkan::CreateIndexBuffer(u64 sizeBytes)
			{
				GPUBuffer_Vulkan* buffer = m_indexBuffers.CreateResource();
				return buffer;
			}

			void RenderContext_Vulkan::FreeVertexBuffer(GPUBuffer* buffer)
			{
				m_vertexBuffers.FreeResource(static_cast<GPUBuffer_Vulkan*>(buffer));
			}

			void RenderContext_Vulkan::FreeIndexBuffer(GPUBuffer* buffer)
			{
				m_indexBuffers.FreeResource(static_cast<GPUBuffer_Vulkan*>(buffer));
			}

			vk::Instance RenderContext_Vulkan::CreateInstance()
			{
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
				return vk::createInstance(instanceCreateInfo);
			}

			vk::PhysicalDevice RenderContext_Vulkan::FindAdapter()
			{
				std::vector<vk::PhysicalDevice> physicalDevices = m_instnace.enumeratePhysicalDevices();
				vk::PhysicalDevice adapter(nullptr);
				for (auto& gpu : physicalDevices)
				{
					adapter = gpu;
					if (adapter.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
					{
						break;
					}
				}
				return adapter;
			}

			std::vector<vk::DeviceQueueCreateInfo> RenderContext_Vulkan::GetDeviceQueueCreateInfos(std::vector<QueueInfo>& queueInfo)
			{
				std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = {};
				std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_adapter.getQueueFamilyProperties();
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

			void RenderContext_Vulkan::GetDeviceExtensionAndLayers(std::vector<std::string>& extensions, std::vector<std::string>& layers)
			{
				std::vector<vk::LayerProperties> layerProperties = m_adapter.enumerateDeviceLayerProperties();
				std::vector<vk::ExtensionProperties> extensionProperties = m_adapter.enumerateDeviceExtensionProperties();

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
					IS_CORE_INFO("Device extensions:");
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
			
			vk::SwapchainKHR RenderContext_Vulkan::CreateSwapchain()
			{
				vk::SurfaceCapabilitiesKHR surfaceCapabilites = m_adapter.getSurfaceCapabilitiesKHR(m_surface);
				const int imageCount = std::max(c_FrameCount, (int)surfaceCapabilites.minImageCount);

				vk::Extent2D swapchainExtent = {};
				// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
				if (surfaceCapabilites.currentExtent == vk::Extent2D{ 0xFFFFFFFF, 0xFFFFFFFF })
				{
					// If the surface size is undefined, the size is set to
					// the size of the images requested.
					swapchainExtent.width = Window::Instance().GetWidth();
					swapchainExtent.height = Window::Instance().GetHeight();
				}
				else
				{
					// If the surface size is defined, the swap chain size must match
					swapchainExtent = surfaceCapabilites.currentExtent;
				}

				// Select a present mode for the swapchain

				std::vector<vk::PresentModeKHR> presentModes = m_adapter.getSurfacePresentModesKHR(m_surface);
				// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
				// This mode waits for the vertical blank ("v-sync")
				vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
				if (true)
				{
					for (size_t i = 0; i < presentModes.size(); ++i)
					{
						if (presentModes[i] == vk::PresentModeKHR::eMailbox)
						{
							presentMode = vk::PresentModeKHR::eMailbox;
							break;
						}
						if (presentMode != vk::PresentModeKHR::eMailbox && presentModes[i] == vk::PresentModeKHR::eImmediate)
						{
							presentMode = vk::PresentModeKHR::eImmediate;
						}
					}
				}

				vk::ImageUsageFlags imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
				const vk::ImageUsageFlagBits imageUsageBits[] =
				{
					vk::ImageUsageFlagBits::eTransferSrc,
					vk::ImageUsageFlagBits::eTransferDst
				};
				for (const auto& flag : imageUsageBits)
				{
					if (surfaceCapabilites.supportedUsageFlags & flag)
					{
						imageUsage |= flag;
					}
				}

				vk::Format surfaceFormat;
				vk::ColorSpaceKHR surfaceColourSpace;
				std::vector<vk::SurfaceFormatKHR> formats = m_adapter.getSurfaceFormatsKHR(m_surface);
				// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
				// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
				if ((formats.size() == 1) && (formats[0].format == vk::Format::eUndefined))
				{
					surfaceFormat = vk::Format::eR8G8B8A8Unorm;
					surfaceColourSpace = formats[0].colorSpace;
				}
				else
				{
					// iterate over the list of available surface format and
					// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
					bool found_B8G8R8A8_UNORM = false;
					for (auto&& format : formats)
					{
						if (format.format == vk::Format::eB8G8R8A8Unorm)// VK_FORMAT_B8G8R8A8_UNORM)
						{
							surfaceFormat = format.format;
							surfaceColourSpace = format.colorSpace;
							found_B8G8R8A8_UNORM = true;
							break;
						}
					}

					// in case VK_FORMAT_B8G8R8A8_UNORM is not available
					// select the first available color format
					if (!found_B8G8R8A8_UNORM)
					{
						surfaceFormat = formats[0].format;
						surfaceColourSpace = formats[0].colorSpace;
					}
				}
				m_swapchainFormat = surfaceFormat;

				vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR();
				createInfo.surface = m_surface;
				createInfo.setMinImageCount(static_cast<u32>(imageCount));
				createInfo.setImageFormat(m_swapchainFormat);
				createInfo.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
				createInfo.setImageExtent({ swapchainExtent });
				createInfo.setImageArrayLayers(static_cast<u32>(1));
				createInfo.setImageUsage(imageUsage);
				createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
				createInfo.setPresentMode(presentMode);
				createInfo.setOldSwapchain(m_swapchain);

				vk::SwapchainKHR swapchain =  m_device.createSwapchainKHR(createInfo);

				if (m_swapchain)
				{
					for (vk::ImageView& view : m_swapchainImageViews)
					{
						m_device.destroyImageView(view);
					}
					m_swapchainImageViews.clear();
					m_device.destroySwapchainKHR(m_swapchain);
					m_swapchain = nullptr;
				}

				m_swapchainImages = m_device.getSwapchainImagesKHR(swapchain);
				for (vk::Image& image : m_swapchainImages)
				{
					vk::ImageViewCreateInfo info = vk::ImageViewCreateInfo(
						{},
						image,
						vk::ImageViewType::e2D,
						m_swapchainFormat);
					info.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
					m_swapchainImageViews.push_back(m_device.createImageView(info));
				}

				return swapchain;
			}

			void RenderContext_Vulkan::FrameResource::Init(RenderContext_Vulkan* context)
			{
				Context = context;

				CommandPool.Init(context);

				vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();
				SwapchainAcquire = Context->GetDevice().createSemaphore(semaphoreInfo);
				SignalSemaphore = Context->GetDevice().createSemaphore(semaphoreInfo);

				vk::FenceCreateInfo fenceCreateInfo = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
				SubmitFence = context->GetDevice().createFence(fenceCreateInfo);
			}

			void RenderContext_Vulkan::FrameResource::Destroy()
			{
				CommandPool.Destroy();

				Context->GetDevice().destroySemaphore(SwapchainAcquire);
				Context->GetDevice().destroySemaphore(SignalSemaphore);
				Context->GetDevice().destroyFence(SubmitFence);
			}
}
	}
}