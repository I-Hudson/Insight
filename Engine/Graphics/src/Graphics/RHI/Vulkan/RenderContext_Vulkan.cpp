#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/Window.h"

#include "Core/Logger.h"
#include "Core/MemoryTracker.h"

#include "Core/Profiler.h"
#include "Platforms/Platform.h"

#include "Event/EventSystem.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#ifdef IS_PLATFORM_WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             /// Exclude rarely-used stuff from Windows headers.
#endif
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#include <VmaUsage.h>

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

				#if VK_KHR_shader_float16_int8 && VK_VERSION_1_2 == 0
				VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
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

			static void* VMAVulkanAllocate(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
			{
				return NewBytes(size, Core::MemoryAllocCategory::Graphics);
			}
			static void* VMAVulkanReallocate(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
			{
				Core::MemoryTracker::Instance().UnTrack(pOriginal);
				void* newBlock = std::realloc(pOriginal, size);
				Core::MemoryTracker::Instance().Track(newBlock, size, Core::MemoryAllocCategory::Graphics, Core::MemoryTrackAllocationType::Single);
				return newBlock;
			}
			static void VMAVulkanFree(void* pUserData, void* pMemory)
			{
				DeleteBytes(pMemory);
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

			PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
			PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
			PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
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
					IS_LOG_CORE_ERROR("Id: {}\n Name: {}\n Msg: {}", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
				}

				/// The return value of this callback controls whether the Vulkan call that caused the validation message will be aborted or not
				/// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message to abort
				/// If you instead want to have calls abort, pass in VK_TRUE and the function will return VK_ERROR_VALIDATION_FAILED_EXT 
				return VK_FALSE;
			}

			bool RenderContext_Vulkan::Init(RenderContextDesc desc)
			{
				IS_PROFILE_FUNCTION();

				std::lock_guard lock(m_lock);
				m_desc = desc;

				if (m_instnace && m_device)
				{
					IS_LOG_CORE_ERROR("[RenderContext_Vulkan::Init] Context already inited.");
					return true;
				}

				CreateInstance();

				vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instnace, "vkCreateDebugUtilsMessengerEXT"));
				vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instnace, "vkDestroyDebugUtilsMessengerEXT"));
				vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(GetExtensionFunction("vkSetDebugUtilsObjectNameEXT"));

				VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
				debugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
				debugUtilsMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
				debugUtilsMessengerCI.pfnUserCallback = DebugUtilsMessengerCallback;
				ASSERT_MSG(vkCreateDebugUtilsMessengerEXT, "[RenderContext_Vulkan::Init] Unable to call 'vkCreateDebugUtilsMessengerEXT'.");
				if (vkCreateDebugUtilsMessengerEXT)
				{
					ThrowIfFailed(vkCreateDebugUtilsMessengerEXT(m_instnace, &debugUtilsMessengerCI, nullptr, &debugUtilsMessenger));
				}

				m_adapter = FindAdapter();

				m_gpuCrashTracker = RHI_GPUCrashTracker::Create();
				if (m_gpuCrashTracker)
				{
					m_gpuCrashTracker->Init();
				}

				std::vector<QueueInfo> queueInfo = {};
				std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos = GetDeviceQueueCreateInfos(queueInfo);
				int queueSize = 0;
				for (size_t i = 0; i < deviceQueueCreateInfos.size(); ++i)
				{
					VkDeviceQueueCreateInfo& createInfo = deviceQueueCreateInfos[i];
					queueSize += createInfo.queueCount;
				}

				std::vector<float> queuePriorities;
				queuePriorities.reserve(queueSize);
				queueSize = 0;
				for (size_t i = 0; i < deviceQueueCreateInfos.size(); ++i)
				{
					VkDeviceQueueCreateInfo& createInfo = deviceQueueCreateInfos[i];
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

				VkPhysicalDeviceVulkan13Features deviceFeaturesToEnable13 = { };
				deviceFeaturesToEnable13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

				VkPhysicalDeviceVulkan12Features deviceFeaturesToEnable12 = { };
				deviceFeaturesToEnable12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
				deviceFeaturesToEnable12.pNext = &deviceFeaturesToEnable13;

				VkPhysicalDeviceFeatures2 deviceFeaturesToEnable = { };
				deviceFeaturesToEnable.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				vkGetPhysicalDeviceFeatures2(m_adapter, &deviceFeaturesToEnable);
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

				VkDeviceCreateInfo deviceCreateInfo = { };
				deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				deviceCreateInfo.pNext = &deviceFeaturesToEnable;
				deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
				deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(deviceQueueCreateInfos.size());
				deviceCreateInfo.ppEnabledLayerNames = deviceLayersCC.data();
				deviceCreateInfo.enabledLayerCount = static_cast<u32>(deviceLayersCC.size());
				deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionsCC.data();
				deviceCreateInfo.enabledExtensionCount = static_cast<u32>(deviceExtensionsCC.size());
				deviceCreateInfo.pEnabledFeatures = nullptr;
				ThrowIfFailed(vkCreateDevice(m_adapter, &deviceCreateInfo, nullptr, &m_device));

				SetObjectName(m_physical_device_info.Device_Name.c_str(), (u64)m_adapter, VK_OBJECT_TYPE_PHYSICAL_DEVICE);
				SetObjectName(m_physical_device_info.Device_Name, (u64)m_device, VK_OBJECT_TYPE_DEVICE);

				for (size_t i = 0; i < queueInfo.size(); ++i)
				{
					const QueueInfo& info = queueInfo[i];
					VkQueue queue;
					vkGetDeviceQueue(m_device, info.FamilyQueueIndex, info.FamilyQueueIndex, &queue);
					m_commandQueues[info.Queue] = queue;
					m_queueFamilyLookup[info.Queue] = info.FamilyQueueIndex;
				}

				VkSurfaceKHR surfaceKHR;
				VkResult res = glfwCreateWindowSurface(m_instnace, Window::Instance().GetRawWindow(), nullptr, &surfaceKHR);
				m_surface = VkSurfaceKHR(surfaceKHR);

				m_pipelineLayoutManager.SetRenderContext(this);
				m_pipelineManager.SetRenderContext(this);
				m_renderpassManager.SetRenderContext(this);

				/// Initialise vulkan memory allocator
				m_vmaAllocationCallbacks.pfnAllocation = VMAVulkanAllocate;
				m_vmaAllocationCallbacks.pfnReallocation = VMAVulkanReallocate;
				m_vmaAllocationCallbacks.pfnFree = VMAVulkanFree;
				
				VmaAllocatorCreateInfo allocatorInfo{};
				allocatorInfo.instance = m_instnace;
				allocatorInfo.physicalDevice = m_adapter;
				allocatorInfo.device = m_device;
				allocatorInfo.pAllocationCallbacks = &m_vmaAllocationCallbacks;

				ThrowIfFailed(vmaCreateAllocator(&allocatorInfo, &m_vmaAllocator));

				char* stats;
				vmaBuildStatsString(m_vmaAllocator, &stats, true);
				IS_LOG_CORE_INFO("VMA: {}.", stats);
				Delete(stats);
				//VmaFree(m_vmaAllocator, stats);

				{
					std::array<VkDescriptorPoolSize, 2> pool_sizes =
					{
						VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	1024 },
						VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			1024 },
					};

					VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { };
					descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
					descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
					descriptorPoolCreateInfo.maxSets = 2048 * 4;
					descriptorPoolCreateInfo.pPoolSizes = pool_sizes.data();
					descriptorPoolCreateInfo.poolSizeCount = static_cast<u32>(pool_sizes.size());
					ThrowIfFailed(vkCreateDescriptorPool(m_device, &descriptorPoolCreateInfo, nullptr, &m_descriptor_pool));

					m_commandListManager.ForEach([this](CommandListManager& manager)
						{
							manager.Create(this);
						});
				}

				m_submitFrameContexts.Setup();
				m_submitFrameContexts.ForEach([this](FrameSubmitContext& context)
					{
						VkFenceCreateInfo fenceCreateInfo = {};
						fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
						fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
						ThrowIfFailed(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &context.SubmitFences));

						VkSemaphoreCreateInfo semaphoreCreateInfo = {};
						semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
						ThrowIfFailed(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &context.SignalSemaphores));
						ThrowIfFailed(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &context.SwapchainAcquires));
					});

				m_uploadQueue.Init();

				if (desc.MultithreadContext)
				{
					StartRenderThread();
				}

				return true;
			}

			void RenderContext_Vulkan::Destroy()
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_lock);

				GpuWaitForIdle();
				m_resource_tracker.Release();

				DestroyImGui();

				m_submitFrameContexts.ForEach([this](FrameSubmitContext& context)
					{
						vkDestroyFence(m_device, context.SubmitFences, nullptr);
						context.SubmitFences = nullptr;
						vkDestroySemaphore(m_device, context.SignalSemaphores, nullptr);
						context.SignalSemaphores = nullptr;
						vkDestroySemaphore(m_device, context.SwapchainAcquires, nullptr);
						context.SwapchainAcquires = nullptr;
					});

				if (m_swapchain)
				{
					for (RHI_Texture*& tex : m_swapchainImages)
					{
						static_cast<RHI_Texture_Vulkan*>(tex)->m_image = VkImage();
						tex->Release();
						Renderer::FreeTexture(tex);
						tex = nullptr;
					}
					m_swapchainImages.clear();
					vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
					m_swapchain = nullptr;
					m_swapchainImages.resize(0);
				}

				BaseDestroy();

				vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
				m_descriptor_pool = nullptr;

				m_pipelineManager.Destroy();
				m_pipelineLayoutManager.Destroy();

				if (m_surface)
				{
					vkDestroySurfaceKHR(m_instnace, m_surface, nullptr);
					m_surface = nullptr;
				}

				vmaDestroyAllocator(m_vmaAllocator);
				m_vmaAllocator = VK_NULL_HANDLE;

				vkDestroyDevice(m_device, nullptr);
				m_device = nullptr;

				if (m_gpuCrashTracker)
				{
					m_gpuCrashTracker->Destroy();
					DeleteTracked(m_gpuCrashTracker);
				}

				if (debugUtilsMessenger)
				{
					ASSERT_MSG(vkDestroyDebugUtilsMessengerEXT, "[RenderContext_Vulkan::Destroy] 'vkDestroyDebugUtilsMessengerEXT' is null.")
					if (vkDestroyDebugUtilsMessengerEXT)
					{
						vkDestroyDebugUtilsMessengerEXT(m_instnace, debugUtilsMessenger, nullptr);
					}
					debugUtilsMessenger = nullptr;
				}

				if (vkCreateDebugUtilsMessengerEXT)
				{
					vkCreateDebugUtilsMessengerEXT = nullptr;
				}
				if (vkDestroyDebugUtilsMessengerEXT)
				{
					vkDestroyDebugUtilsMessengerEXT = nullptr;
				}
				if (vkSetDebugUtilsObjectNameEXT)
				{
					vkSetDebugUtilsObjectNameEXT = nullptr;
				}

				vkDestroyInstance(m_instnace, nullptr);
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
				ThrowIfFailed(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_imguiDescriptorPool));

				// Create renderpass
				RenderpassDescription renderpassDescription = { };
				renderpassDescription.AddAttachment(AttachmentDescription::Load(m_swapchainDesc.Format, Graphics::ImageLayout::PresentSrc));
				renderpassDescription.Attachments.back().InitalLayout = Graphics::ImageLayout::ColourAttachment;
				renderpassDescription.AllowDynamicRendering = false;
				renderpassDescription.SwapchainPass = true;
				RHI_Renderpass renderpass = GetRenderpassManager().GetOrCreateRenderpass(renderpassDescription);
				SetObjectName("ImguiRenderPass", (u64)renderpass.Resource, VK_OBJECT_TYPE_RENDER_PASS);

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
				init_info.MinImageCount = RenderContext::Instance().GetFramesInFligtCount();
				init_info.ImageCount = RenderContext::Instance().GetFramesInFligtCount();
				init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
				init_info.Allocator = nullptr;
				init_info.CheckVkResultFn = [](VkResult error)
				{
					if (error != VK_SUCCESS)
					{
						IS_LOG_CORE_ERROR("[IMGUI] Error: {}", static_cast<u32>(error));
					}
				};
				ImGui_ImplVulkan_Init(&init_info, static_cast<VkRenderPass>(renderpass.Resource));
				ImGuiBeginFrame();
			}

			void RenderContext_Vulkan::DestroyImGui()
			{
				IS_PROFILE_FUNCTION();

				ImGuiRelease();
				ImGui_ImplVulkan_Shutdown();

				for (VkFramebuffer& frameBuffer : m_imguiFramebuffers)
				{
					vkDestroyFramebuffer(m_device, frameBuffer, nullptr);
					frameBuffer = nullptr;
				}

				vkDestroyDescriptorPool(m_device, m_imguiDescriptorPool, nullptr);
				m_imguiDescriptorPool = nullptr;
			}

			bool RenderContext_Vulkan::PrepareRender()
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_lock);

				if (Window::Instance().GetWidth() == 0 || Window::Instance().GetHeight() == 0)
				{
					return false;
				}

				if (Window::Instance().GetSize() != m_swapchainBufferSize)
				{
					IS_PROFILE_SCOPE("Swapchain resize");
					SetSwaphchainResolution(Maths::Vector2(Window::Instance().GetWidth(), Window::Instance().GetHeight()));
					return false;
				}

				{
					IS_PROFILE_SCOPE("Fence wait");
					// First get the status of the fence. Then if it has not finished, wait on it.
					
					VkResult fenceStatusResult = vkWaitForFences(m_device, 1, &m_submitFrameContexts.Get().SubmitFences, 1, 0);
					if (fenceStatusResult == VK_TIMEOUT)
					{						
						VkResult waitResult = vkWaitForFences(m_device, 1, &m_submitFrameContexts.Get().SubmitFences, 1, INFINITE);
						ASSERT(waitResult == VK_SUCCESS);
					}
					m_frameIndexCompleted.store(m_frameIndex.load());
				}

				{
					IS_PROFILE_SCOPE("acquireNextImageKHR");
					VkResult acquireNextImageResult = 
						vkAcquireNextImageKHR(m_device, static_cast<VkSwapchainKHR>(m_swapchain), INFINITE, static_cast<VkSemaphore>(m_submitFrameContexts.Get().SwapchainAcquires)
						, 0, &m_availableSwapchainImage);

					vkResetFences(m_device, 1, &m_submitFrameContexts.Get().SubmitFences);
					for (const RHI_CommandList* cmdList : m_submitFrameContexts.Get().CommandLists)
					{
						if (cmdList)
						{
							cmdList->OnWorkCompleted();
						}
					}

					if (acquireNextImageResult != VK_SUCCESS)
					{
						SetSwaphchainResolution(Maths::Vector2(Window::Instance().GetWidth(),Window::Instance().GetHeight()));
						return false;
					}
				}

				m_descriptorSetManager->Reset();
				m_commandListManager->Reset();

				m_resource_tracker.BeginFrame();

				return true;
			}

			void RenderContext_Vulkan::PreRender(RHI_CommandList* cmdList)
			{
				ExecuteAsyncJobs(cmdList);
			}

			void RenderContext_Vulkan::PostRender(RHI_CommandList* cmdList)
			{
				m_submitFrameContexts.Get().CommandLists.clear();

				if (cmdList != nullptr)
				{
					RHI_CommandList_Vulkan* cmdListVulkan = static_cast<RHI_CommandList_Vulkan*>(cmdList);

					std::array<VkSemaphore, 1> waitSemaphores = { m_submitFrameContexts.Get().SwapchainAcquires };
					std::array<VkPipelineStageFlags, 1> dstStageFlgs = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
					std::array<VkCommandBuffer, 1> commandBuffers = { cmdListVulkan->GetCommandList() };
					std::array<VkSemaphore, 1> signalSemaphore = { m_submitFrameContexts.Get().SignalSemaphores };

					VkSubmitInfo submitInfo = {};
					submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
					submitInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
					submitInfo.pWaitSemaphores = waitSemaphores.data();
					submitInfo.pWaitDstStageMask = dstStageFlgs.data();
					submitInfo.commandBufferCount = static_cast<u32>(commandBuffers.size());
					submitInfo.pCommandBuffers = commandBuffers.data();
					submitInfo.signalSemaphoreCount = static_cast<u32>(signalSemaphore.size());
					submitInfo.pSignalSemaphores = signalSemaphore.data();

					std::array<VkSemaphore, 1> presentWaitSemaphores = { m_submitFrameContexts.Get().SignalSemaphores };
					std::array<VkSwapchainKHR, 1> presentSwapchains = { m_swapchain };
					std::array<u32, 1> presentSwapchainImageIndex = { (u32)m_availableSwapchainImage };

					VkPresentInfoKHR presentInfo = {};
					presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
					presentInfo.waitSemaphoreCount = static_cast<u32>(presentWaitSemaphores.size());
					presentInfo.pWaitSemaphores = presentWaitSemaphores.data();
					presentInfo.swapchainCount = static_cast<u32>(presentSwapchains.size());
					presentInfo.pSwapchains = presentSwapchains.data();
					presentInfo.pImageIndices = presentSwapchainImageIndex.data();

					{
						IS_PROFILE_SCOPE("Present");
						std::lock_guard lock(m_lock);
						if (!cmdList->IsDiscard())
						{
							m_submitFrameContexts.Get().CommandLists.push_back(cmdList);

							RHI_CommandList_Vulkan* cmdListVulkan = static_cast<RHI_CommandList_Vulkan*>(cmdList);
							cmdListVulkan->m_state = RHI_CommandListStates::Submitted;

							vkQueueSubmit(m_commandQueues[GPUQueue_Graphics], 1, &submitInfo, m_submitFrameContexts.Get().SubmitFences);
							VkResult presentResult = vkQueuePresentKHR(m_commandQueues[GPUQueue_Graphics], &presentInfo);

							if (presentResult != VK_SUCCESS)
							{
								SetSwaphchainResolution(Maths::Vector2(Window::Instance().GetWidth(), Window::Instance().GetHeight()));
							}
							m_currentFrame = (m_currentFrame + 1) % RenderContext::Instance().GetFramesInFligtCount();
							m_frameIndex = m_currentFrame;
						}
					}
				}

				{
					IS_PROFILE_SCOPE("ImGui NewFrame");
					ImGui_ImplVulkan_NewFrame();
					//ImGuiBeginFrame();
				}

				m_resource_tracker.EndFrame();
			}

			void RenderContext_Vulkan::CreateSwapchain(SwapchainDesc desc)
			{
				IS_PROFILE_FUNCTION();

				VkSurfaceCapabilitiesKHR surfaceCapabilites = { };
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_adapter, m_surface, &surfaceCapabilites);
				const int imageCount = (int)std::max(RenderContext::Instance().GetFramesInFligtCount(), surfaceCapabilites.minImageCount);

				VkExtent2D swapchainExtent = { };
				// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
				if (surfaceCapabilites.currentExtent.width == 0xFFFFFFFF
					&& surfaceCapabilites.currentExtent.height == 0xFFFFFFFF)
				{
					// If the surface size is undefined, the size is set to
					// the size of the images requested.
					swapchainExtent.width = desc.Width;
					swapchainExtent.height = desc.Height;
				}
				else
				{
					// If the surface size is defined, the swap chain size must match
					swapchainExtent = surfaceCapabilites.currentExtent;
				}
				m_swapchainBufferSize = Maths::Vector2(swapchainExtent.width, swapchainExtent.height);

				// Select a present mode for the swapchain

				u32 presentModeCount = 0;
				vkGetPhysicalDeviceSurfacePresentModesKHR(m_adapter, m_surface, &presentModeCount, nullptr);

				std::vector<VkPresentModeKHR> presentModes;
				presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(m_adapter, m_surface, &presentModeCount, presentModes.data());

				// Try and find our desired present mode. 
				VkPresentModeKHR desiredPresentMode = SwapchainPresentModeToVulkan(desc.PresentMode);
				VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				for (size_t i = 0; i < presentModes.size(); ++i)
				{
					if (presentModes[i] == desiredPresentMode)
					{
						presentMode = desiredPresentMode;
						break;
					}
				}
				

				VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				const VkImageUsageFlagBits imageUsageBits[] =
				{
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
					VK_IMAGE_USAGE_TRANSFER_DST_BIT
				};
				for (const auto& flag : imageUsageBits)
				{
					if (surfaceCapabilites.supportedUsageFlags & flag)
					{
						imageUsage |= flag;
					}
				}

				u32 surfaceFormatCount = 0;
				vkGetPhysicalDeviceSurfaceFormatsKHR(m_adapter, m_surface, &surfaceFormatCount, nullptr);

				std::vector<VkSurfaceFormatKHR> formats;
				formats.resize(surfaceFormatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(m_adapter, m_surface, &surfaceFormatCount, formats.data());

				bool isValidFormat = false;
				for (auto const& format : formats)
				{
					if (format.format == PixelFormatToVulkan(desc.Format))
					{
						m_swapchainFormat = PixelFormatToVulkan(desc.Format);
						isValidFormat = true;
						break;
					}
				}
				if (!isValidFormat)
				{
					m_swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
				}

				VkSwapchainCreateInfoKHR swapchainCreateInfo = { };
				swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapchainCreateInfo.surface = m_surface;
				swapchainCreateInfo.minImageCount = static_cast<u32>(imageCount);
				swapchainCreateInfo.imageFormat = m_swapchainFormat;
				swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
				swapchainCreateInfo.imageExtent = swapchainExtent;
				swapchainCreateInfo.imageArrayLayers = 1ul;
				swapchainCreateInfo.imageUsage = imageUsage;
				swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
				swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				swapchainCreateInfo.presentMode = presentMode;
				swapchainCreateInfo.oldSwapchain = m_swapchain;

				VkSwapchainKHR swapchain;
				ThrowIfFailed(vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &swapchain));

				if (m_swapchain)
				{
					for (RHI_Texture*& tex : m_swapchainImages)
					{
						static_cast<RHI_Texture_Vulkan*>(tex)->m_image = VkImage();
						tex->Release();
						Renderer::FreeTexture(tex);
						tex = nullptr;
					}
					m_swapchainImages.clear();
					vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
					m_swapchain = nullptr;
				}

				u32 swapchainImageCount = 0;
				vkGetSwapchainImagesKHR(m_device, swapchain, &swapchainImageCount, nullptr);

				std::vector<VkImage> swapchainImages;
				swapchainImages.resize(swapchainImageCount);
				vkGetSwapchainImagesKHR(m_device, swapchain, &swapchainImageCount, swapchainImages.data());

				int image_index = 0;
				for (VkImage& image : swapchainImages)
				{
					VkImageSubresourceRange imageSubresourceRange = { };
					imageSubresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
					imageSubresourceRange.baseMipLevel = 0;
					imageSubresourceRange.levelCount = 1;
					imageSubresourceRange.baseArrayLayer = 0;
					imageSubresourceRange.layerCount = 1;

					VkImageViewCreateInfo viewCreateInfo = { };
					viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					viewCreateInfo.image = image;
					viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					viewCreateInfo.format = m_swapchainFormat;
					viewCreateInfo.components = VkComponentMapping();
					viewCreateInfo.subresourceRange = imageSubresourceRange;

					// We create two views into the same image as this is how the API is designed.
					// The first view (image_view), is used for all layers.
					// The second view (single_layer_image_view) is used for a single layer. In this case the first layer.
					VkImageView image_view;
					ThrowIfFailed(vkCreateImageView(m_device, &viewCreateInfo, nullptr, &image_view));
					VkImageView single_layer_image_view;
					ThrowIfFailed(vkCreateImageView(m_device, &viewCreateInfo, nullptr, &single_layer_image_view));

					RHI_Texture* tex = Renderer::CreateTexture();
					tex->SetName("Swapchain_Image: " + std::to_string(image_index++));

					RHI_TextureInfo texCreateInfo = { };
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
					for (size_t i = 0; i < texCreateInfo.Mip_Count; ++i)
					{
						texVulkan->m_infos.push_back(texCreateInfo);
					}

					m_swapchainImages.push_back(tex);
				}
				m_swapchain = swapchain;
				m_swapchainDesc = desc;
			}

			void RenderContext_Vulkan::SetSwaphchainResolution(Maths::Vector2 resolution)
			{
				WaitForGpu();

				SwapchainDesc desc = m_swapchainDesc;
				desc.Width = static_cast<u32>(resolution.x);
				desc.Height = static_cast<u32>(resolution.y);
				CreateSwapchain(desc);

				Core::EventSystem::Instance().DispatchEvent(MakeRPtr<Core::GraphcisSwapchainResize>(m_swapchainBufferSize.x, m_swapchainBufferSize.y));
			}

			Maths::Vector2 RenderContext_Vulkan::GetSwaphchainResolution() const
			{
				return m_swapchainBufferSize;
			}

			void RenderContext_Vulkan::GpuWaitForIdle()
			{
				IS_PROFILE_FUNCTION();

				vkDeviceWaitIdle(m_device);
			}

			void RenderContext_Vulkan::SubmitCommandListAndWait(RHI_CommandList* cmdList)
			{
				IS_PROFILE_FUNCTION();
				const RHI_CommandList_Vulkan* cmdListVulkan = static_cast<RHI_CommandList_Vulkan*>(cmdList);

				std::array<VkCommandBuffer, 1> commandBuffers = { cmdListVulkan->GetCommandList() };
				
				VkSubmitInfo submitInfo = {};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.waitSemaphoreCount = 0;
				submitInfo.pWaitSemaphores = nullptr;
				submitInfo.pWaitDstStageMask = 0;
				submitInfo.commandBufferCount = static_cast<u32>(commandBuffers.size());
				submitInfo.pCommandBuffers = commandBuffers.data();
				submitInfo.signalSemaphoreCount = 0;
				submitInfo.pSignalSemaphores = nullptr;

				VkFenceCreateInfo fenceCreateInfo = {};
				fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

				VkFence waitFence;
				ThrowIfFailed(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &waitFence));

				std::lock_guard lock(m_command_queue_mutexs[GPUQueue_Graphics]);
				
				ThrowIfFailed(vkQueueSubmit(m_commandQueues[GPUQueue_Graphics], 1, &submitInfo, waitFence));

				ThrowIfFailed(vkWaitForFences(m_device, 1, &waitFence, 1, INFINITE));
				vkDestroyFence(m_device, waitFence, nullptr);
				waitFence = nullptr;
			}

			void RenderContext_Vulkan::ExecuteAsyncJobs(RHI_CommandList* cmdList)
			{
				// Go through out defered manager and call all the functions which have been queued up.
				m_gpu_defered_manager.Update(cmdList);
				m_uploadQueue.UploadToDevice(cmdList);
			}

			void RenderContext_Vulkan::SetObjectName(std::string_view name, u64 handle, VkObjectType objectType)
			{
				IS_PROFILE_FUNCTION();

				VkDebugUtilsObjectNameInfoEXT info = {};
				info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
				info.objectType = objectType;
				info.objectHandle = handle;
				info.pObjectName = name.data();

				if (vkSetDebugUtilsObjectNameEXT)
				{
					vkSetDebugUtilsObjectNameEXT(m_device, &info);
				}
			}

			RHI_Texture* RenderContext_Vulkan::GetSwaphchainIamge() const
			{
				return m_swapchainImages[m_availableSwapchainImage];
			}

			VkImageView RenderContext_Vulkan::GetSwapchainImageView() const
			{
				return static_cast<RHI_Texture_Vulkan*>(m_swapchainImages[m_availableSwapchainImage])->GetImageView();
			}

			void* RenderContext_Vulkan::GetExtensionFunction(const char* function)
			{
				return vkGetInstanceProcAddr(m_instnace, function);
			}

			void RenderContext_Vulkan::WaitForGpu()
			{
				IS_PROFILE_FUNCTION();
				vkDeviceWaitIdle(m_device);
			}

			void RenderContext_Vulkan::CreateInstance()
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

				VkApplicationInfo applicationInfo = { };
				applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				applicationInfo.pApplicationName = "ApplciationName";
				applicationInfo.applicationVersion = 0;
				applicationInfo.pEngineName = "Insight";
				applicationInfo.engineVersion = 0;
				applicationInfo.apiVersion = std::min(sdk_version, driver_version);

				std::vector<const char*> enabledLayerNames;

				if (m_desc.GPUValidation)
				{
					enabledLayerNames.push_back("VK_LAYER_KHRONOS_validation");
				}

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

				VkInstanceCreateInfo instanceCreateInfo = { };
				instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				instanceCreateInfo.pNext = nullptr;
				instanceCreateInfo.flags = 0;
				instanceCreateInfo.pApplicationInfo = &applicationInfo;
				instanceCreateInfo.enabledLayerCount = static_cast<u32>(enabledLayerNames.size());
				instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
				instanceCreateInfo.enabledExtensionCount = static_cast<u32>(enabledExtensionNames.size());
				instanceCreateInfo.ppEnabledExtensionNames = enabledExtensionNames.data();

#if defined(VK_EXT_validation_features) && defined(_DEBUG)
				std::vector<VkValidationFeatureEnableEXT> validation_features_enabled;
				validation_features_enabled.push_back(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);
				///validation_features_enabled.push_back(VkValidationFeatureEnableEXT::eSynchronizationValidation);
				///validation_features_enabled.push_back(VkValidationFeatureEnableEXT::eGpuAssisted);

				VkValidationFeaturesEXT validation_features{};
				validation_features.pNext = nullptr;
				validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
				validation_features.enabledValidationFeatureCount = static_cast<u32>(validation_features_enabled.size());
				validation_features.pEnabledValidationFeatures = validation_features_enabled.data();
				//instanceCreateInfo.pNext = &validation_features;
#endif
				ThrowIfFailed(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instnace));
			}

			VkPhysicalDevice RenderContext_Vulkan::FindAdapter()
			{
				IS_PROFILE_FUNCTION();
				
				u32 physicalDeviceCount = 0;
				vkEnumeratePhysicalDevices(m_instnace, &physicalDeviceCount, nullptr);

				std::vector<VkPhysicalDevice> physicalDevices;
				physicalDevices.resize(physicalDeviceCount);
				vkEnumeratePhysicalDevices(m_instnace, &physicalDeviceCount, physicalDevices.data());

				VkPhysicalDevice adapter(nullptr);
				for (auto& gpu : physicalDevices)
				{
					adapter = gpu;
					VkPhysicalDeviceProperties deviceProperties;
					vkGetPhysicalDeviceProperties(adapter, &deviceProperties);
					if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					{
						break;
					}
				}

				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(adapter, &deviceProperties);
				VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
				vkGetPhysicalDeviceMemoryProperties(adapter, &deviceMemoryProperties);

				m_physical_device_info.Device_Name = deviceProperties.deviceName;
				m_physical_device_info.Vendor_Id = deviceProperties.vendorID;
				m_physical_device_info.VRam_Size += deviceMemoryProperties.memoryHeaps[0].size;
				m_physical_device_info.SetVendorName();

				m_physical_device_info.MinUniformBufferAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;

				return adapter;
		}

			std::vector<VkDeviceQueueCreateInfo> RenderContext_Vulkan::GetDeviceQueueCreateInfos(std::vector<QueueInfo>& queueInfo)
			{
				u32 queueFmailyPropertiesCount = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(m_adapter, &queueFmailyPropertiesCount, nullptr);

				std::vector<VkQueueFamilyProperties> queueFamilyProperties;
				queueFamilyProperties.resize(queueFmailyPropertiesCount);
				vkGetPhysicalDeviceQueueFamilyProperties(m_adapter, &queueFmailyPropertiesCount, queueFamilyProperties.data());

				std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};
				int graphicsQueue = -1;
				int computeQueue = -1;
				int transferQueue = -1;

				for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
				{
					const VkQueueFamilyProperties& queueProp = queueFamilyProperties[i];

					VkDeviceQueueCreateInfo queueCreateInfo = {};
					queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queueCreateInfo.queueFamilyIndex = static_cast<u32>(i);
					queueCreateInfo.queueCount = queueProp.queueCount;

					if ((queueProp.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
					{
						queueInfo.push_back(QueueInfo{ static_cast<int>(i), GPUQueue::GPUQueue_Graphics });
						graphicsQueue = static_cast<int>(i);
					}
					if ((queueProp.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT && computeQueue == -1 && graphicsQueue != i)
					{
						queueInfo.push_back(QueueInfo{ static_cast<int>(i), GPUQueue::GPUQueue_Compute });
						computeQueue = static_cast<int>(i);
					}
					if ((queueProp.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT &&
						transferQueue == -1 && (queueProp.queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT &&
						(queueProp.queueFlags & VK_QUEUE_COMPUTE_BIT) != VK_QUEUE_COMPUTE_BIT)
					{
						queueInfo.push_back(QueueInfo{ static_cast<int>(i), GPUQueue::GPUQueue_Transfer });
						transferQueue = static_cast<int>(i);
					}
					queueCreateInfos.push_back(queueCreateInfo);
				}

				return queueCreateInfos;
			}

			void RenderContext_Vulkan::GetDeviceExtensionAndLayers(std::set<std::string>& extensions, std::set<std::string>& layers, bool includeAll)
			{
				u32 propertyCount = 0;
				vkEnumerateDeviceLayerProperties(m_adapter, &propertyCount, nullptr);

				std::vector<VkLayerProperties> layerProperties;
				layerProperties.resize(propertyCount);
				vkEnumerateDeviceLayerProperties(m_adapter, &propertyCount, layerProperties.data());

				u32 extensionCount = 0;
				vkEnumerateDeviceExtensionProperties(m_adapter, nullptr, &extensionCount, nullptr);

				std::vector<VkExtensionProperties> extensionProperties;
				extensionProperties.resize(extensionCount);
				vkEnumerateDeviceExtensionProperties(m_adapter, nullptr, &extensionCount, extensionProperties.data());

				IS_LOG_CORE_INFO("Device layers:");
				for (size_t i = 0; i < layerProperties.size(); ++i)
				{
					IS_LOG_CORE_INFO("{}", layerProperties[i].layerName);
				}
				IS_LOG_CORE_INFO("Device extensions:");
				for (size_t i = 0; i < extensionProperties.size(); ++i)
				{
					IS_LOG_CORE_INFO("{}", extensionProperties[i].extensionName);
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
						if (std::find_if(extensionProperties.begin(), extensionProperties.end(), [ext](const VkExtensionProperties& extnesion)
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

			void RenderContext_Vulkan::SetDeviceExtensions()
			{
				std::set<std::string> deviceExts;
				std::set<std::string> layerExts;
				GetDeviceExtensionAndLayers(deviceExts, layerExts, true);

				VkPhysicalDeviceFeatures features{};
				vkGetPhysicalDeviceFeatures(m_adapter, &features);

				VkPhysicalDeviceShaderFloat16Int8Features native16Bit{};

				VkPhysicalDeviceFeatures2 features2{};
				features2.pNext = &native16Bit;
				vkGetPhysicalDeviceFeatures2(m_adapter, &features2);

				m_deviceExtensions[(u8)DeviceExtension::BindlessDescriptors] = deviceExts.find(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) != deviceExts.end();
				m_deviceExtensions[(u8)DeviceExtension::ExclusiveFullScreen] = deviceExts.find(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) != deviceExts.end();
				m_deviceExtensions[(u8)DeviceExtension::VulkanDynamicRendering] = deviceExts.find(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) != deviceExts.end();
				m_deviceExtensions[(u8)DeviceExtension::FormatTypeCasting] = true;
				m_deviceExtensions[(u8)DeviceExtension::Native16BitOps] = native16Bit.shaderFloat16;
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
					u32 extensionCount = 0;
					vkEnumerateDeviceExtensionProperties(m_adapter, nullptr, &extensionCount, nullptr);

					std::vector<VkExtensionProperties> extensions;
					extensions.resize(extensionCount);
					vkEnumerateDeviceExtensionProperties(m_adapter, nullptr, &extensionCount, extensions.data());

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