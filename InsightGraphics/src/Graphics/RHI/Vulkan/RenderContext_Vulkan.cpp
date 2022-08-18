#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#include "Graphics/Window.h"
#include "Core/Logger.h"

#include "Core/Profiler.h"

#include "Event/EventManager.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#ifdef IS_PLATFORM_WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
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

				// The return value of this callback controls whether the Vulkan call that caused the validation message will be aborted or not
				// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message to abort
				// If you instead want to have calls abort, pass in VK_TRUE and the function will return VK_ERROR_VALIDATION_FAILED_EXT 
				return VK_FALSE;
			}

			bool RenderContext_Vulkan::Init()
			{
				IS_PROFILE_FUNCTION();

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

				const vk::PhysicalDeviceFeatures deviceFeatures = m_adapter.getFeatures();

				vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo({}, deviceQueueCreateInfos, deviceLayersCC, deviceExtensionsCC, &deviceFeatures);

				vk::PhysicalDeviceDescriptorIndexingFeaturesEXT physicalDeviceDescriptorIndexingFeatures = {};
				if (HasExtension(DeviceExtension::BindlessDescriptors))
				{
					physicalDeviceDescriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
					deviceCreateInfo.setPNext(&physicalDeviceDescriptorIndexingFeatures);
				}

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

				// Initialise vulkan memory allocator
				VmaAllocatorCreateInfo allocatorInfo{};
				allocatorInfo.instance = m_instnace;
				allocatorInfo.physicalDevice = m_adapter;
				allocatorInfo.device = m_device;
				ThrowIfFailed(vmaCreateAllocator(&allocatorInfo, &m_vmaAllocator));

				for (FrameResource_Vulkan& frame : m_frames)
				{
					frame.Init(this);
				}

				InitImGui();

				return true;
			}

			void RenderContext_Vulkan::Destroy()
			{
				IS_PROFILE_FUNCTION();

				m_device.waitIdle();

				DestroyImGui();

				for (FrameResource_Vulkan& frame : m_frames)
				{
					frame.Destroy();
				}

				BaseDestroy();

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
				//1: create descriptor pool for IMGUI
				// the size of the pool is very oversize, but it's copied from imgui demo itself.
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

				// Create the Render Pass
				{
					VkAttachmentDescription attachment = {};
					attachment.format = (VkFormat)m_swapchainFormat;
					attachment.samples = VK_SAMPLE_COUNT_1_BIT;
					attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					VkAttachmentReference color_attachment = {};
					color_attachment.attachment = 0;
					color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					VkSubpassDescription subpass = {};
					subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
					subpass.colorAttachmentCount = 1;
					subpass.pColorAttachments = &color_attachment;
					VkSubpassDependency dependency = {};
					dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
					dependency.dstSubpass = 0;
					dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					dependency.srcAccessMask = 0;
					dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					VkRenderPassCreateInfo info = {};
					info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
					info.attachmentCount = 1;
					info.pAttachments = &attachment;
					info.subpassCount = 1;
					info.pSubpasses = &subpass;
					info.dependencyCount = 1;
					info.pDependencies = &dependency;
					m_imguiRenderpass = m_device.createRenderPass(info);

					// Render imgui straight on top of what ever is on the swap chain image.
				}

				// Setup Platform/Renderer backends
				ImGui_ImplGlfw_InitForVulkan(Window::Instance().GetRawWindow(), true);
				ImGui_ImplVulkan_InitInfo init_info = {};
				init_info.Instance = m_instnace;
				init_info.PhysicalDevice = m_adapter;
				init_info.Device = m_device;
				init_info.QueueFamily = m_queueFamilyLookup[GPUQueue_Graphics];
				init_info.Queue = m_commandQueues[GPUQueue_Graphics];
				init_info.PipelineCache = nullptr;
				init_info.DescriptorPool = m_imguiDescriptorPool;
				init_info.Subpass = 0;
				init_info.MinImageCount = c_FrameCount;
				init_info.ImageCount = c_FrameCount;
				init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
				init_info.Allocator = nullptr;
				init_info.CheckVkResultFn = [](VkResult error)
				{
					if (error != 0)
					{
						IS_CORE_ERROR("[IMGUI] Error: {}", error);
					}
					};
				ImGui_ImplVulkan_Init(&init_info, m_imguiRenderpass);

				FrameResource_Vulkan& frame = m_frames[m_currentFrame];
				frame.CommandListManager.Update();
				RHI_CommandList_Vulkan* cmdListVulkan = dynamic_cast<RHI_CommandList_Vulkan*>(frame.CommandListManager.GetCommandList());

				ImGui_ImplVulkan_CreateFontsTexture(cmdListVulkan->GetCommandList());

				cmdListVulkan->GetCommandList().end();

				std::array<vk::CommandBuffer, 1> commandBuffers = { cmdListVulkan->GetCommandList() };
				vk::SubmitInfo submitInfo = vk::SubmitInfo();
				submitInfo.setCommandBuffers(commandBuffers);
				m_commandQueues[GPUQueue_Graphics].submit(submitInfo);

				m_device.waitIdle();

				ImGui_ImplVulkan_DestroyFontUploadObjects();
				frame.CommandListManager.Update();

				ImGui_ImplVulkan_NewFrame();
				ImGuiBeginFrame();
			}

			void RenderContext_Vulkan::DestroyImGui()
			{
				IS_PROFILE_FUNCTION();

				ImGui_ImplVulkan_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext();

				for (vk::Framebuffer& frameBuffer : m_imguiFramebuffers)
				{
					m_device.destroyFramebuffer(frameBuffer);
					frameBuffer = nullptr;
				}

				m_device.destroyDescriptorPool(m_imguiDescriptorPool);
				m_imguiDescriptorPool = nullptr;
				m_device.destroyRenderPass(m_imguiRenderpass);
				m_imguiRenderpass = nullptr;
			}

			void RenderContext_Vulkan::Render(CommandList cmdList)
			{
				IS_PROFILE_FUNCTION();

				if (Window::Instance().GetSize() != m_swapchainBufferSize)
				{
					IS_PROFILE_SCOPE("Swapchain resize");
					WaitForGpu();
					CreateSwapchain();
					Core::EventManager::Instance().DispatchEvent(MakeRPtr<Core::GraphcisSwapchainResize>(m_swapchainBufferSize.x, m_swapchainBufferSize.y));
					return;
				}

				{
					IS_PROFILE_SCOPE("ImGui Render");
					ImGuiRender();
				}
				FrameResource_Vulkan& frame = m_frames[m_currentFrame];

				{
					IS_PROFILE_SCOPE("Fence wait");
					vk::Result waitResult = m_device.waitForFences({ frame.SubmitFence }, 1, 0xFFFFFFFF);
					assert(waitResult == vk::Result::eSuccess);
				}

				vk::ResultValue nextImage = m_device.acquireNextImageKHR(m_swapchain, 0xFFFFFFFF, frame.SwapchainAcquire);
				m_availableSwapchainImage = nextImage.value;
				m_device.resetFences({ frame.SubmitFence });

				frame.Reset();
				PrepareRender();

				RHI_CommandList_Vulkan* cmdListVulkan = dynamic_cast<RHI_CommandList_Vulkan*>(frame.CommandListManager.GetCommandList());
				cmdListVulkan->Record(cmdList, &frame);
				cmdListVulkan->GetCommandList().endRenderPass();

				// Render imgui.
				if (m_imguiFramebuffers[m_currentFrame])
				{
					m_device.destroyFramebuffer(m_imguiFramebuffers[m_currentFrame]);
				}

				std::array<vk::ImageView, 1> imguiFramebufferViews = { GetSwapchainImageView() };
				vk::FramebufferCreateInfo frameBufferInfo = vk::FramebufferCreateInfo({},
					m_imguiRenderpass,
					imguiFramebufferViews,
					m_swapchainBufferSize.x,
					m_swapchainBufferSize.y, 1);
				m_imguiFramebuffers[m_currentFrame] = m_device.createFramebuffer(frameBufferInfo);

				VkClearValue clearValue;
				clearValue.color.float32[0] = 0;
				clearValue.color.float32[1] = 0;
				clearValue.color.float32[2] = 0;
				clearValue.color.float32[3] = 1;

				{
					IS_PROFILE_SCOPE("ImGui Draw");
					VkRenderPassBeginInfo info = {};
					info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					info.renderPass = m_imguiRenderpass;
					info.framebuffer = m_imguiFramebuffers[m_currentFrame];
					info.renderArea.extent.width = m_swapchainBufferSize.x;
					info.renderArea.extent.height = m_swapchainBufferSize.y;
					info.clearValueCount = 1;
					info.pClearValues = &clearValue;
					cmdListVulkan->GetCommandList().beginRenderPass(info, vk::SubpassContents::eInline);
					// Record dear imgui primitives into command buffer
					ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdListVulkan->GetCommandList());
					cmdListVulkan->GetCommandList().endRenderPass();
					cmdListVulkan->GetCommandList().end();
				}
				std::array<vk::Semaphore, 1> waitSemaphores = { frame.SwapchainAcquire };
				std::array<vk::PipelineStageFlags, 1> dstStageFlgs = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
				std::array<vk::CommandBuffer, 1> commandBuffers = { cmdListVulkan->GetCommandList() };
				std::array<vk::Semaphore, 1> signalSemaphore = { frame.SignalSemaphore };
			}

			void RenderContext_Vulkan::PrepareRender()
			{
#ifdef RENDER_GRAPH_ENABLED
				IS_PROFILE_FUNCTION();

				if (Window::Instance().GetSize() != m_swapchainBufferSize)
				{
					IS_PROFILE_SCOPE("Swapchain resize");
					WaitForGpu();
					CreateSwapchain();
					Core::EventManager::Instance().DispatchEvent(MakeRPtr<Core::GraphcisSwapchainResize>(m_swapchainBufferSize.x, m_swapchainBufferSize.y));
					return;
				}

				{
					IS_PROFILE_SCOPE("ImGui Render");
					ImGuiRender();
				}

				{
					IS_PROFILE_SCOPE("Fence wait");
					vk::Result waitResult = m_device.waitForFences({ *m_submitFences.Get() }, 1, 0xFFFFFFFF);
					assert(waitResult == vk::Result::eSuccess);
				}

				vk::ResultValue nextImage = m_device.acquireNextImageKHR(m_swapchain, 0xFFFFFFFF, *m_swapchainAcquires.Get() );
				m_availableSwapchainImage = nextImage.value;
				m_device.resetFences({ *m_submitFences.Get() });
#endif
			}

			void RenderContext_Vulkan::PostRender()
			{
				vk::SubmitInfo submitInfo = vk::SubmitInfo(
					waitSemaphores,
					dstStageFlgs,
					commandBuffers,
					signalSemaphore);
				{
					IS_PROFILE_SCOPE("Submit");
					m_commandQueues[GPUQueue_Graphics].submit(submitInfo, frame.SubmitFence);
				}
				std::array<vk::Semaphore, 1> signalSemaphores = { frame.SignalSemaphore };
				std::array<vk::SwapchainKHR, 1> swapchains = { m_swapchain };
				std::array<u32, 1> swapchainImageIndex = { (u32)m_availableSwapchainImage };

				vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR(signalSemaphores, swapchains, swapchainImageIndex);
				{
					IS_PROFILE_SCOPE("Present");

					vk::Result presentResult = m_commandQueues[GPUQueue_Graphics].presentKHR(presentInfo);
				}
				m_currentFrame = (m_currentFrame + 1) % c_FrameCount;

				{
					IS_PROFILE_SCOPE("ImGui NewFrame");
					ImGui_ImplVulkan_NewFrame();
					ImGuiBeginFrame();
				}
			}

			void RenderContext_Vulkan::GpuWaitForIdle()
			{
				m_device.waitIdle();
			}

			void RenderContext_Vulkan::SubmitCommandListAndWait(RHI_CommandList* cmdList)
			{
				IS_PROFILE_FUNCTION();
				const RHI_CommandList_Vulkan* cmdListVulkan = dynamic_cast<RHI_CommandList_Vulkan*>(cmdList);

				std::array<vk::CommandBuffer, 1> commandBuffers = { cmdListVulkan->GetCommandList() };
				vk::SubmitInfo submitInfo = vk::SubmitInfo(
					{ },
					{ },
					commandBuffers,
					{ });
				vk::Fence waitFence = m_device.createFence(vk::FenceCreateInfo());
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

			void RenderContext_Vulkan::WaitForGpu()
			{
				IS_PROFILE_FUNCTION();

				m_device.waitIdle();
			}

			vk::Instance RenderContext_Vulkan::CreateInstance()
			{
				IS_PROFILE_FUNCTION();

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
					VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif

#if VK_KHR_get_surface_capabilities2
					VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
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

#if defined(_DEBUG)
				const char* vkLayerKhronosValidation = "VK_LAYER_KHRONOS_validation";
				bool hasKhronosStandardValidationLayer = std::find_if(layerProperties.begin(), layerProperties.end(), [vkLayerKhronosValidation](const vk::LayerProperties& layer)
					{
						return strcmp(layer.layerName, vkLayerKhronosValidation);
					}) != layerProperties.end();

				if (hasKhronosStandardValidationLayer)
				{
					if (true/*(bool)CONFIG_VAL(Config::GraphicsConfig.Validation)*/)
					{
						layers.insert(vkLayerKhronosValidation);
					}
				}
#endif
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
							extensions.insert(ext);
						}
					}
				}
			}
			
			void RenderContext_Vulkan::CreateSwapchain()
			{
				IS_PROFILE_FUNCTION();

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
				m_swapchainBufferSize = { swapchainExtent.width, swapchainExtent.height };

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
				m_swapchain = swapchain;
			}

			void RenderContext_Vulkan::SetDeviceExtensions()
			{
				std::set<std::string> deviceExts;
				std::set<std::string> layerExts;
				GetDeviceExtensionAndLayers(deviceExts, layerExts, true);

				m_deviceExtensions[(u8)DeviceExtension::BindlessDescriptors] = deviceExts.find(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) != deviceExts.end();
				m_deviceExtensions[(u8)DeviceExtension::ExclusiveFullScreen] = deviceExts.find(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) != deviceExts.end();
			}

			void FrameResource_Vulkan::Init(RenderContext_Vulkan* context)
			{
				Context = context;

				CommandListManager.Create(context);
				DescriptorAllocator.SetRenderContext(Context);
				UniformBuffer.Create(Context);

				vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();
				SwapchainAcquire = Context->GetDevice().createSemaphore(semaphoreInfo);
				SignalSemaphore = Context->GetDevice().createSemaphore(semaphoreInfo);

				vk::FenceCreateInfo fenceCreateInfo = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
				SubmitFence = context->GetDevice().createFence(fenceCreateInfo);
			}

			void FrameResource_Vulkan::Destroy()
			{
				IS_PROFILE_FUNCTION();
				CommandListManager.Destroy();
				DescriptorAllocator.Destroy();
				UniformBuffer.Release();

				Context->GetDevice().destroySemaphore(SwapchainAcquire);
				Context->GetDevice().destroySemaphore(SignalSemaphore);
				Context->GetDevice().destroyFence(SubmitFence);
			}
			
			void FrameResource_Vulkan::Reset()
			{
				IS_PROFILE_FUNCTION();
				FrameResouce::Reset();
				DescriptorAllocator.Reset();
			}
}
	}
}

#endif //#if defined(IS_VULKAN_ENABLED)