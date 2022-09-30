#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/Window.h"
#include "Core/Logger.h"

#include "Core/Profiler.h"

#include "Event/EventManager.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#ifdef IS_PLATFORM_WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             /// Exclude rarely-used stuff from Windows headers.
#endif
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
				///VK_EXT_VALIDATION_CACHE_EXTENSION_NAME,
				#endif

				#if VK_KHR_sampler_mirror_clamp_to_edge && (VK_VERSION_1_2 == 0)
				VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
				#endif

				#if VK_KHR_maintenance3 && (VK_VERSION_1_1 == 0)
				VK_KHR_MAINTENANCE3_EXTENSION_NAME,
				#endif

				#if VK_EXT_descriptor_indexing && (VK_VERSION_1_2 == 0)
				VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
				#endif

				#if VK_KHR_dynamic_rendering && VK_VERSION_1_3 == 0
				VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
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
					Platform::MemClear(&Layer, sizeof(Layer));
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
			vk::DispatchLoaderDynamic debugDispatcher;

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

				/// The return value of this callback controls whether the Vulkan call that caused the validation message will be aborted or not
				/// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message to abort
				/// If you instead want to have calls abort, pass in VK_TRUE and the function will return VK_ERROR_VALIDATION_FAILED_EXT 
				return VK_FALSE;
			}

			bool RenderContext_Vulkan::Init()
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_lock);

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

				std::set<std::string> deviceLayersSets;
				std::set<std::string> deviceExtensionsSets;
				GetDeviceExtensionAndLayers(deviceExtensionsSets, deviceLayersSets);

				SetDeviceExtensions();

				std::vector<std::string> deviceLayers = std::vector<std::string>(deviceLayersSets.begin(), deviceLayersSets.end());
				std::vector<std::string> deviceExtensions = std::vector<std::string>(deviceExtensionsSets.begin(), deviceExtensionsSets.end());

				if (HasExtension(DeviceExtension::ExclusiveFullScreen))
				{
					deviceExtensions.push_back(DeviceExtensionToVulkan(DeviceExtension::ExclusiveFullScreen));
				}

				std::vector<const char*> deviceLayersCC = StringVectorToConstChar(deviceLayers);
				std::vector<const char*> deviceExtensionsCC = StringVectorToConstChar(deviceExtensions);

				vk::PhysicalDeviceVulkan13Features deviceFeaturesToEnable13 = { };

				vk::PhysicalDeviceVulkan12Features deviceFeaturesToEnable12 = { };
				deviceFeaturesToEnable12.pNext = &deviceFeaturesToEnable13;

				vk::PhysicalDeviceFeatures2 deviceFeaturesToEnable = m_adapter.getFeatures2();
				deviceFeaturesToEnable.pNext = &deviceFeaturesToEnable12;

				if (HasExtension(DeviceExtension::BindlessDescriptors))
				{
					deviceFeaturesToEnable12.descriptorBindingPartiallyBound = VK_TRUE;
					deviceFeaturesToEnable12.descriptorIndexing = VK_TRUE;
					deviceFeaturesToEnable12.samplerMirrorClampToEdge = VK_TRUE;
					EnableExtension(DeviceExtension::BindlessDescriptors);
				}
				if (HasExtension(DeviceExtension::VulkanDynamicRendering))
				{
					deviceFeaturesToEnable13.dynamicRendering = VK_TRUE;
					EnableExtension(DeviceExtension::VulkanDynamicRendering);
				}

				vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo({}, deviceQueueCreateInfos, deviceLayersCC, deviceExtensionsCC, nullptr, &deviceFeaturesToEnable);
				m_device = m_adapter.createDevice(deviceCreateInfo);

				vk::DynamicLoader dl;
				PFN_vkGetInstanceProcAddr getInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
				debugDispatcher = vk::DispatchLoaderDynamic(m_instnace, getInstanceProcAddr, m_device);

				for (size_t i = 0; i < queueInfo.size(); ++i)
				{
					const QueueInfo& info = queueInfo[i];
					m_commandQueues[info.Queue] = m_device.getQueue(info.FamilyQueueIndex, info.FamilyQueueIndex);
					m_queueFamilyLookup[info.Queue] = info.FamilyQueueIndex;
				}

				VkSurfaceKHR surfaceKHR;
				VkResult res = glfwCreateWindowSurface(m_instnace, Window::Instance().GetRawWindow(), nullptr, &surfaceKHR);
				m_surface = vk::SurfaceKHR(surfaceKHR);
				CreateSwapchain();

				m_pipelineLayoutManager.SetRenderContext(this);
				m_pipelineStateObjectManager.SetRenderContext(this);
				m_renderpassManager.SetRenderContext(this);

				/// Initialise vulkan memory allocator
				VmaAllocatorCreateInfo allocatorInfo{};
				allocatorInfo.instance = m_instnace;
				allocatorInfo.physicalDevice = m_adapter;
				allocatorInfo.device = m_device;
				ThrowIfFailed(vmaCreateAllocator(&allocatorInfo, &m_vmaAllocator));

				char* stats;
				vmaBuildStatsString(m_vmaAllocator, &stats, true);

				{
					std::array<vk::DescriptorPoolSize, 2> pool_sizes =
					{
						vk::DescriptorPoolSize{ vk::DescriptorType::eCombinedImageSampler,	1024 },
						vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBuffer,			1024 },
					};

					vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = { };
					descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
					descriptorPoolCreateInfo.maxSets = 2048 * 4;
					descriptorPoolCreateInfo.setPoolSizes(pool_sizes);
					m_descriptor_pool = *reinterpret_cast<VkDescriptorPool*>(&m_device.createDescriptorPool(descriptorPoolCreateInfo));

					m_commandListManager.ForEach([this](CommandListManager& manager)
						{
							manager.Create(this);
						});
				}

				InitImGui();

#ifdef RENDER_GRAPH_ENABLED
				m_submitFences.Setup();
				m_submitFences.ForEach([this](vk::Fence& fence)
					{
						fence = m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
					});
				m_swapchainAcquires.Setup();
				m_swapchainAcquires.ForEach([this](vk::Semaphore& semaphore)
					{
						semaphore = m_device.createSemaphore(vk::SemaphoreCreateInfo());
					});
				m_signalSemaphores.Setup();
				m_signalSemaphores.ForEach([this](vk::Semaphore& semaphore)
					{
						semaphore = m_device.createSemaphore(vk::SemaphoreCreateInfo());
					});
#endif

				return true;
			}

			void RenderContext_Vulkan::Destroy()
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_lock);

				m_device.waitIdle();

				DestroyImGui();

#ifdef RENDER_GRAPH_ENABLED
				m_submitFences.ForEach([this](vk::Fence& fence)
					{
						m_device.destroyFence(fence);
					});
				m_swapchainAcquires.ForEach([this](vk::Semaphore& semaphore)
					{
						m_device.destroySemaphore(semaphore);
					});
				m_signalSemaphores.ForEach([this](vk::Semaphore& semaphore)
					{
						m_device.destroySemaphore(semaphore);
					});
#endif

				if (m_swapchain)
				{
					for (RHI_Texture*& tex : m_swapchainImages)
					{
						static_cast<RHI_Texture_Vulkan*>(tex)->m_image = vk::Image();
						tex->Release();
						Renderer::FreeTexture(tex);
					}
					m_swapchainImages.clear();
					m_device.destroySwapchainKHR(m_swapchain);
					m_swapchainImages.resize(0);
				}

				BaseDestroy();

				m_device.destroyDescriptorPool(*reinterpret_cast<VkDescriptorPool*>(&m_descriptor_pool));
				m_descriptor_pool = nullptr;

				m_pipelineStateObjectManager.Destroy();
				m_pipelineLayoutManager.Destroy();

				if (m_surface)
				{
					m_instnace.destroySurfaceKHR(m_surface);
				}

				vmaDestroyAllocator(m_vmaAllocator);
				m_vmaAllocator = VK_NULL_HANDLE;

				m_device.destroy();
				m_device = nullptr;

				if (debugUtilsMessenger)
				{
					vkDestroyDebugUtilsMessengerEXT(m_instnace, debugUtilsMessenger, nullptr);
				}

				m_instnace.destroy();
				m_instnace = nullptr;
			}

			void RenderContext_Vulkan::InitImGui()
			{
				///1: create descriptor pool for IMGUI
				/// the size of the pool is very oversize, but it's copied from imgui demo itself.
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
				m_imguiDescriptorPool = m_device.createDescriptorPool(pool_info);

				/// Setup Platform/Renderer backends
				ImGui_ImplGlfw_InitForVulkan(Window::Instance().GetRawWindow(), false);
				ImGui_ImplVulkan_InitInfo init_info = {};
				init_info.Instance = m_instnace;
				init_info.PhysicalDevice = m_adapter;
				init_info.Device = m_device;
				init_info.QueueFamily = m_queueFamilyLookup[GPUQueue_Graphics];
				init_info.Queue = m_commandQueues[GPUQueue_Graphics];
				init_info.PipelineCache = nullptr;
				init_info.DescriptorPool = m_imguiDescriptorPool;
				init_info.Subpass = 0;
				init_info.MinImageCount = RenderGraph::s_FarmeCount;
				init_info.ImageCount = RenderGraph::s_FarmeCount;
				init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
				init_info.Allocator = nullptr;
				init_info.CheckVkResultFn = [](VkResult error)
				{
					if (error != 0)
					{
						IS_CORE_ERROR("[IMGUI] Error: {}", error);
					}
				};
				if (IsExtensionEnabled(DeviceExtension::VulkanDynamicRendering))
				{
					ImGui_ImplVulkan_Init(&init_info, nullptr);
				}
				else
				{
					ImGui_ImplVulkan_Init(&init_info, m_imguiRenderpass);
				}

				RHI_CommandList_Vulkan* cmdListVulkan = static_cast<RHI_CommandList_Vulkan*>(m_commandListManager->GetCommandList());

				ImGui_ImplVulkan_CreateFontsTexture(cmdListVulkan->GetCommandList());

				cmdListVulkan->GetCommandList().end();

				std::array<vk::CommandBuffer, 1> commandBuffers = { cmdListVulkan->GetCommandList() };
				vk::SubmitInfo submitInfo = vk::SubmitInfo();
				submitInfo.setCommandBuffers(commandBuffers);
				m_commandQueues[GPUQueue_Graphics].submit(submitInfo);

				m_device.waitIdle();

				ImGui_ImplVulkan_DestroyFontUploadObjects();
				m_commandListManager->ReturnCommandList(cmdListVulkan);
				m_commandListManager->Reset();

				ImGuiBeginFrame();
			}

			void RenderContext_Vulkan::DestroyImGui()
			{
				IS_PROFILE_FUNCTION();

				ImGui_ImplVulkan_Shutdown();
				ImGui_ImplGlfw_Shutdown();
			

				for (vk::Framebuffer& frameBuffer : m_imguiFramebuffers)
				{
					m_device.destroyFramebuffer(frameBuffer);
					frameBuffer = nullptr;
				}

				m_device.destroyDescriptorPool(m_imguiDescriptorPool);
				m_imguiDescriptorPool = nullptr;
			}

			bool RenderContext_Vulkan::PrepareRender()
			{
#ifdef RENDER_GRAPH_ENABLED
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_lock);

				if (Window::Instance().GetWidth() == 0 || Window::Instance().GetHeight() == 0)
				{
					return false;
				}

				if (Window::Instance().GetSize() != m_swapchainBufferSize)
				{
					IS_PROFILE_SCOPE("Swapchain resize");
					WaitForGpu();
					CreateSwapchain();
					Core::EventManager::Instance().DispatchEvent(MakeRPtr<Core::GraphcisSwapchainResize>(m_swapchainBufferSize.x, m_swapchainBufferSize.y));
					return false;
				}

				{
					IS_PROFILE_SCOPE("ImGui Render");
					ImGuiRender();
				}

				{
					IS_PROFILE_SCOPE("Fence wait");
					vk::Result waitResult = m_device.waitForFences({ m_submitFences.Get() }, 1, 0xFFFFFFFF);
					assert(waitResult == vk::Result::eSuccess);
				}

				{
					IS_PROFILE_SCOPE("acquireNextImageKHR");
					vk::ResultValue nextImage = m_device.acquireNextImageKHR(m_swapchain, 0xFFFFFFFF, m_swapchainAcquires.Get());
					m_availableSwapchainImage = nextImage.value;
					m_device.resetFences({ m_submitFences.Get() });
				}
#endif
				m_descriptorSetManager->Reset();
				m_commandListManager->Reset();

				return true;
			}

			void RenderContext_Vulkan::PostRender(RHI_CommandList* cmdList)
			{
#ifdef RENDER_GRAPH_ENABLED
				std::lock_guard lock(m_lock);

				RHI_CommandList_Vulkan* cmdListVulkan = static_cast<RHI_CommandList_Vulkan*>(cmdList);

				std::array<vk::Semaphore, 1> waitSemaphores = { m_swapchainAcquires.Get() };
				std::array<vk::PipelineStageFlags, 1> dstStageFlgs = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
				std::array<vk::CommandBuffer, 1> commandBuffers = { cmdListVulkan->GetCommandList() };
				std::array<vk::Semaphore, 1> signalSemaphore = { m_signalSemaphores.Get() };

				vk::SubmitInfo submitInfo = vk::SubmitInfo(
					waitSemaphores,
					dstStageFlgs,
					commandBuffers,
					signalSemaphore);
				{
					IS_PROFILE_SCOPE("Submit");
					m_commandQueues[GPUQueue_Graphics].submit(submitInfo, m_submitFences.Get());
				}
				std::array<vk::Semaphore, 1> signalSemaphores = { m_signalSemaphores.Get() };
				std::array<vk::SwapchainKHR, 1> swapchains = { m_swapchain };
				std::array<u32, 1> swapchainImageIndex = { (u32)m_availableSwapchainImage };

				vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR(signalSemaphores, swapchains, swapchainImageIndex);
				{
					IS_PROFILE_SCOPE("Present");

					vk::Result presentResult = m_commandQueues[GPUQueue_Graphics].presentKHR(presentInfo);
				}
				m_currentFrame = (m_currentFrame + 1) % RenderGraph::s_FarmeCount;

				{
					IS_PROFILE_SCOPE("ImGui NewFrame");
					ImGui_ImplVulkan_NewFrame();
					ImGuiBeginFrame();
				}
#endif
			}

			void RenderContext_Vulkan::GpuWaitForIdle()
			{
				m_device.waitIdle();
			}

			void RenderContext_Vulkan::SubmitCommandListAndWait(RHI_CommandList* cmdList)
			{
				IS_PROFILE_FUNCTION();
				const RHI_CommandList_Vulkan* cmdListVulkan = static_cast<RHI_CommandList_Vulkan*>(cmdList);

				std::array<vk::CommandBuffer, 1> commandBuffers = { cmdListVulkan->GetCommandList() };
				vk::SubmitInfo submitInfo = vk::SubmitInfo(
					{ },
					{ },
					commandBuffers,
					{ });
				vk::Fence waitFence = m_device.createFence(vk::FenceCreateInfo());
				std::lock_guard lock(m_command_queue_mutexs[GPUQueue_Graphics]);
				m_commandQueues[GPUQueue_Graphics].submit(submitInfo, waitFence);
				vk::Result waitForFenceResult = m_device.waitForFences({ waitFence }, 1, INFINITE);
				m_device.destroyFence(waitFence);
			}

			void RenderContext_Vulkan::SetObejctName(std::wstring_view name, u64 handle, vk::ObjectType objectType)
			{
				IS_PROFILE_FUNCTION();

				std::string str;
				std::transform(name.begin(), name.end(), std::back_inserter(str), [](wchar_t c) {
					return (char)c;
					});

				vk::DebugUtilsObjectNameInfoEXT info = vk::DebugUtilsObjectNameInfoEXT(objectType, handle, str.c_str());
				m_device.setDebugUtilsObjectNameEXT(info, debugDispatcher);
			}

			RHI_Texture* RenderContext_Vulkan::GetSwaphchainIamge() const
			{
				return m_swapchainImages[m_availableSwapchainImage];
			}

			vk::ImageView RenderContext_Vulkan::GetSwapchainImageView() const
			{
				return static_cast<RHI_Texture_Vulkan*>(m_swapchainImages[m_availableSwapchainImage])->GetImageView();
			}

			void RenderContext_Vulkan::WaitForGpu()
			{
				IS_PROFILE_FUNCTION();

				m_device.waitIdle();
			}

			vk::Instance RenderContext_Vulkan::CreateInstance()
			{
				IS_PROFILE_FUNCTION();

				/// Get sdk version
				uint32_t sdk_version = VK_HEADER_VERSION_COMPLETE;

				/// Get driver version
				uint32_t driver_version = 0;
				{
					/// Per LunarG, if vkEnumerateInstanceVersion is not present, we are running on Vulkan 1.0
					/// https:///www.lunarg.com/wp-content/uploads/2019/02/Vulkan-1.1-Compatibility-Statement_01_19.pdf
					auto eiv = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

					if (eiv)
					{
						eiv(&driver_version);
					}
					else
					{
						driver_version = VK_API_VERSION_1_0;
					}
				}

				vk::ApplicationInfo applicationInfo = vk::ApplicationInfo(
					"ApplciationName",
					0,
					"Insight",
					0,
					VK_API_VERSION_1_2);

				applicationInfo.setApiVersion(std::min(sdk_version, driver_version));

				std::vector<const char*> enabledLayerNames =
				{
#ifdef DEBUG
					"VK_LAYER_KHRONOS_validation",
#endif
				};
				std::vector<const char*> enabledExtensionNames;
#if VK_KHR_surface
				if (CheckInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME))
				{
					enabledExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
				}
#endif

#ifdef IS_PLATFORM_WIN32
#if VK_KHR_win32_surface
				if (CheckInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
				{
					enabledExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
				}
#endif
#endif

#if VK_KHR_display
				if (CheckInstanceExtension(VK_KHR_DISPLAY_EXTENSION_NAME))
				{
					enabledExtensionNames.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
				}
#endif


#if VK_KHR_get_display_properties2
				if (CheckInstanceExtension(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME))
				{
					enabledExtensionNames.push_back(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
				}
#endif        

#if VK_EXT_debug_utils
				if (CheckInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
				{
					enabledExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				}
#endif

#if VK_KHR_get_surface_capabilities2
				if (CheckInstanceExtension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME))
				{
					enabledExtensionNames.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
				}
#endif

#if VK_KHR_get_physical_device_properties2 && !VK_VERSION_1_1
				VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#endif

					vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo(
						{ },
						&applicationInfo,
						enabledLayerNames,
						enabledExtensionNames);

#if defined(VK_EXT_validation_features) && defined(_DEBUG)
				std::vector<vk::ValidationFeatureEnableEXT> validation_features_enabled;
				validation_features_enabled.push_back(vk::ValidationFeatureEnableEXT::eBestPractices);
				///validation_features_enabled.push_back(vk::ValidationFeatureEnableEXT::eSynchronizationValidation);
				///validation_features_enabled.push_back(vk::ValidationFeatureEnableEXT::eGpuAssisted);

				vk::ValidationFeaturesEXT validation_features = { };
				validation_features.setEnabledValidationFeatures(validation_features_enabled);
				instanceCreateInfo.setPNext(&validation_features);
#endif
				return vk::createInstance(instanceCreateInfo);
			}

			vk::PhysicalDevice RenderContext_Vulkan::FindAdapter()
			{
				IS_PROFILE_FUNCTION();

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

				vk::PhysicalDeviceProperties properties = adapter.getProperties();
				vk::PhysicalDeviceMemoryProperties memory_properties = adapter.getMemoryProperties();

				m_physical_device_info.Device_Name = std::wstring(properties.deviceName.begin(), properties.deviceName.end());
				m_physical_device_info.Vendor_Id = properties.vendorID;
				m_physical_device_info.VRam_Size += memory_properties.memoryHeaps.at(0).size;
				m_physical_device_info.SetVendorName();

				m_physical_device_info.MinUniformBufferAlignment = properties.limits.minUniformBufferOffsetAlignment;

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

			void RenderContext_Vulkan::GetDeviceExtensionAndLayers(std::set<std::string>& extensions, std::set<std::string>& layers, bool includeAll)
			{
				std::vector<vk::LayerProperties> layerProperties = m_adapter.enumerateDeviceLayerProperties();
				std::vector<vk::ExtensionProperties> extensionProperties = m_adapter.enumerateDeviceExtensionProperties();

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

				if (includeAll)
				{
					for (size_t i = 0; i < extensionProperties.size(); ++i)
					{
						extensions.insert(extensionProperties[i].extensionName);
					}
				}
				else
				{
					for (size_t i = 0; i < ARRAY_COUNT(g_DeviceExtensions); i++)
					{
						const char* ext = g_DeviceExtensions[i];
						if (std::find_if(extensionProperties.begin(), extensionProperties.end(), [ext](const vk::ExtensionProperties& extnesion)
							{
								return strcmp(extnesion.extensionName, ext);
							}) != extensionProperties.end())
						{
							if (CheckForDeviceExtension(ext))
							{
								extensions.insert(ext);
							}
						}
					}
				}
			}

			void RenderContext_Vulkan::CreateSwapchain()
			{
				IS_PROFILE_FUNCTION();

				vk::SurfaceCapabilitiesKHR surfaceCapabilites = m_adapter.getSurfaceCapabilitiesKHR(m_surface);
				const int imageCount = (int)std::max(RenderGraph::s_FarmeCount, surfaceCapabilites.minImageCount);

				vk::Extent2D swapchainExtent = {};
				/// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
				if (surfaceCapabilites.currentExtent == vk::Extent2D{ 0xFFFFFFFF, 0xFFFFFFFF })
				{
					/// If the surface size is undefined, the size is set to
					/// the size of the images requested.
					swapchainExtent.width = Window::Instance().GetWidth();
					swapchainExtent.height = Window::Instance().GetHeight();
				}
				else
				{
					/// If the surface size is defined, the swap chain size must match
					swapchainExtent = surfaceCapabilites.currentExtent;
				}
				m_swapchainBufferSize = { swapchainExtent.width, swapchainExtent.height };

				/// Select a present mode for the swapchain

				std::vector<vk::PresentModeKHR> presentModes = m_adapter.getSurfacePresentModesKHR(m_surface);
				/// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
				/// This mode waits for the vertical blank ("v-sync")
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
				/// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
				/// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
				if ((formats.size() == 1) && (formats[0].format == vk::Format::eUndefined))
				{
					surfaceFormat = vk::Format::eR8G8B8A8Unorm;
					surfaceColourSpace = formats[0].colorSpace;
				}
				else
				{
					/// iterate over the list of available surface format and
					/// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
					bool found_B8G8R8A8_UNORM = false;
					for (auto&& format : formats)
					{
						if (format.format == vk::Format::eB8G8R8A8Unorm)/// VK_FORMAT_B8G8R8A8_UNORM)
						{
							surfaceFormat = format.format;
							surfaceColourSpace = format.colorSpace;
							found_B8G8R8A8_UNORM = true;
							break;
						}
					}

					/// in case VK_FORMAT_B8G8R8A8_UNORM is not available
					/// select the first available color format
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
				vk::SwapchainKHR swapchain = m_device.createSwapchainKHR(createInfo);

				if (m_swapchain)
				{
					for (RHI_Texture*& tex : m_swapchainImages)
					{
						static_cast<RHI_Texture_Vulkan*>(tex)->m_image = vk::Image();
						tex->Release();
						Renderer::FreeTexture(tex);
					}
					m_swapchainImages.clear();
					m_device.destroySwapchainKHR(m_swapchain);
					m_swapchain = nullptr;
				}

				std::vector<vk::Image> swapchainImages = m_device.getSwapchainImagesKHR(swapchain);
				int image_index = 0;
				for (vk::Image& image : swapchainImages)
				{
					vk::ImageViewCreateInfo info = vk::ImageViewCreateInfo(
						{},
						image,
						vk::ImageViewType::e2D,
						m_swapchainFormat);
					info.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

					// We create two views into the same image as this is how the API is designed.
					// The first view (image_view), is used for all layers.
					// The second view (single_layer_image_view) is used for a single layer. In this case the first layer.
					vk::ImageView image_view = m_device.createImageView(info);
					vk::ImageView single_layer_image_view = m_device.createImageView(info);
					
					RHI_Texture* tex = Renderer::CreateTexture();
					tex->SetName(L"Swapchain_Image: " + std::to_wstring(image_index++));

					RHI_TextureCreateInfo texCreateInfo = { };
					texCreateInfo.TextureType = TextureType::Tex2D;
					texCreateInfo.Width = swapchainExtent.width;
					texCreateInfo.Height = swapchainExtent.height;
					texCreateInfo.Depth = 1;
					texCreateInfo.Format = VkFormatToPixelFormat[(int)m_swapchainFormat];
					texCreateInfo.ImageUsage = 0;

					RHI_Texture_Vulkan* texVulkan = static_cast<RHI_Texture_Vulkan*>(tex);
					texVulkan->m_context = this;
					texVulkan->m_image_view = image_view;
					texVulkan->m_single_layer_image_views.push_back(single_layer_image_view);
					texVulkan->m_image = image;
					texVulkan->m_info = texCreateInfo;

					m_swapchainImages.push_back(tex);
				}
				m_swapchain = swapchain;
			}

			void RenderContext_Vulkan::SetDeviceExtensions()
			{
				std::set<std::string> deviceExts;
				std::set<std::string> layerExts;
				GetDeviceExtensionAndLayers(deviceExts, layerExts, true);

				m_deviceExtensions[(u8)DeviceExtension::BindlessDescriptors] = deviceExts.find(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) != deviceExts.end();
				m_deviceExtensions[(u8)DeviceExtension::ExclusiveFullScreen] = deviceExts.find(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) != deviceExts.end();
				m_deviceExtensions[(u8)DeviceExtension::VulkanDynamicRendering] = deviceExts.find(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) != deviceExts.end();
			}

			bool RenderContext_Vulkan::CheckInstanceExtension(const char* extension)
			{
				static std::set<std::string> instanceExtensions;
				if (instanceExtensions.empty())
				{
					uint32_t count;
					vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr); ///get number of extensions
					std::vector<VkExtensionProperties> extensions(count);
					vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()); ///populate buffer
					for (auto& extension : extensions)
					{
						instanceExtensions.insert(extension.extensionName);
					}
				}
				return instanceExtensions.find(extension) != instanceExtensions.end();
			}

			bool RHI::Vulkan::RenderContext_Vulkan::CheckForDeviceExtension(const char* extension)
			{
				static std::set<std::string> deviceExtensions;
				if (deviceExtensions.empty())
				{
					std::vector<vk::ExtensionProperties> extensions = m_adapter.enumerateDeviceExtensionProperties();
					for (auto& extension : extensions)
					{
						deviceExtensions.insert(extension.extensionName);
					}
				}
				return deviceExtensions.find(extension) != deviceExtensions.end();
			}
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)