#include "Graphics/GPU/RHI/Vulkan/GPUSwapchain_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUSemaphore_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUFence_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUCommandList_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/Renderer.h"
#include "Graphics/Window.h"
#include "Core/Logger.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			GPUSwapchain_Vulkan::~GPUSwapchain_Vulkan()
			{
				Destroy();
			}

			void GPUSwapchain_Vulkan::Prepare()
			{	
				VkSurfaceKHR surfaceKHR;
				VkResult res = glfwCreateWindowSurface(GetDevice()->GetInstance(), Window::Instance().GetRawWindow(), nullptr, &surfaceKHR);
				m_surfaceKHR = vk::SurfaceKHR(surfaceKHR);

				vk::PhysicalDevice& physicalDevice = GetDevice()->GetAdapter()->GetPhysicalDevice();

				std::vector<vk::QueueFamilyProperties> queueProperties = physicalDevice.getQueueFamilyProperties();

				const u32 queueCount = static_cast<u32>(queueProperties.size());
				// Iterate over each queue to learn whether it supports presenting:
				// Find a queue with present support
				// Will be used to present the swap chain images to the windowing system
				std::vector<VkBool32> supportsPresent(queueCount);
				for (uint32_t i = 0; i < queueCount; i++)
				{
					supportsPresent[i] = physicalDevice.getSurfaceSupportKHR(i, m_surfaceKHR);
				}

				// Search for a graphics and a present queue in the array of queue
				// families, try to find one that supports both
				uint32_t graphicsQueueNodeIndex = UINT32_MAX;
				uint32_t presentQueueNodeIndex = UINT32_MAX;
				for (uint32_t i = 0; i < queueCount; i++)
				{
					if (queueProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
					{
						if (graphicsQueueNodeIndex == UINT32_MAX)
						{
							graphicsQueueNodeIndex = i;
						}

						if (supportsPresent[i] == VK_TRUE)
						{
							graphicsQueueNodeIndex = i;
							presentQueueNodeIndex = i;
							break;
						}
					}
				}
				if (presentQueueNodeIndex == UINT32_MAX)
				{
					// If there's no queue that supports both present and graphics
					// try to find a separate present queue
					for (uint32_t i = 0; i < queueCount; ++i)
					{
						if (supportsPresent[i] == VK_TRUE)
						{
							presentQueueNodeIndex = i;
							break;
						}
					}
				}

				// Exit if either a graphics or a presenting queue hasn't been found
				if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
				{
					IS_CORE_WARN("[GPUSwapchainVulkan::Init] Could not find a graphics and/or presenting queue!");
				}

				// todo : Add support for separate graphics and presenting queue
				if (graphicsQueueNodeIndex != presentQueueNodeIndex)
				{
					IS_CORE_INFO("[GPUSwapchainVulkan::Init] Separate graphics and presenting queues are not supported yet!");
				}

				vk::Format surfaceFormat;
				vk::ColorSpaceKHR surfaceColourSpace;

				std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(m_surfaceKHR);
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
				m_surfaceFormat = VkFormatToPixelFormat[(int)surfaceFormat];
				m_colourSpace = surfaceColourSpace;

				for (auto* semaphore : m_nextImageAvailable)
				{
					semaphore = GPUSemaphoreManager::Instance().GetOrCreateSemaphore();
				}
				for (auto* semaphore : m_completedImageAvailable)
				{
					semaphore = GPUSemaphoreManager::Instance().GetOrCreateSemaphore();
				}
			}

			void GPUSwapchain_Vulkan::Build(GPUSwapchainDesc desc)
			{
				m_desc = desc;
				vk::SwapchainKHR oldSwapchain = m_swapchain;
				vk::PhysicalDevice& physicalDevice = GetDevice()->GetAdapter()->GetPhysicalDevice();

				vk::SurfaceCapabilitiesKHR surfaceCapabilites = physicalDevice.getSurfaceCapabilitiesKHR(m_surfaceKHR);
				std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(m_surfaceKHR);
			
				vk::Extent2D swapchainExtent = {};
				// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
				if (surfaceCapabilites.currentExtent == vk::Extent2D{ 0xFFFFFFFF, 0xFFFFFFFF })
				{
					// If the surface size is undefined, the size is set to
					// the size of the images requested.
					swapchainExtent.width = m_desc.Width;
					swapchainExtent.height = m_desc.Height;
				}
				else
				{
					// If the surface size is defined, the swap chain size must match
					swapchainExtent = surfaceCapabilites.currentExtent;
					m_desc.Width = surfaceCapabilites.currentExtent.width;
					m_desc.Height = surfaceCapabilites.currentExtent.height;
				}

				// Select a present mode for the swapchain

				// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
				// This mode waits for the vertical blank ("v-sync")
				vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
				if (!m_desc.VSync)
				{
					for (size_t i = 0; i < presentModes.size(); ++i)
					{
						if (m_desc.GSync && presentModes[i] == vk::PresentModeKHR::eMailbox)
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

				// Determine the number of images
				m_desc.ImageCount = std::max(m_desc.ImageCount, (int)surfaceCapabilites.minImageCount);
			
				// Find the transformation of the surface
				vk::SurfaceTransformFlagsKHR preTransform;
				if (surfaceCapabilites.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
				{
					preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
				}
				else
				{
					preTransform = surfaceCapabilites.currentTransform;
				}

				// Find a supported composite alpha format (not all devices support alpha opaque)
				vk::CompositeAlphaFlagsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
				// Simply select the first composite alpha format available
				std::vector<vk::CompositeAlphaFlagsKHR> compositeAlphaFlags = 
				{
					vk::CompositeAlphaFlagBitsKHR::eOpaque,
					vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
					vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
					vk::CompositeAlphaFlagBitsKHR::eInherit,
				};
				for (const auto& compositeAlphaFlag : compositeAlphaFlags)
				{
					if (surfaceCapabilites.supportedCompositeAlpha & compositeAlphaFlag)
					{
						compositeAlpha = compositeAlphaFlag;
						break;
					};
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

				GPUDevice_Vulkan* gpuDevice = GetDevice();
				vk::Device& device = gpuDevice->GetDevice();

				vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR();
				createInfo.surface = m_surfaceKHR;
				createInfo.setMinImageCount(static_cast<u32>(m_desc.ImageCount));
				createInfo.setImageFormat(PixelFormatToVkFormat[(int)m_surfaceFormat]);
				createInfo.setImageColorSpace(m_colourSpace);
				createInfo.setImageExtent(swapchainExtent);
				createInfo.setImageArrayLayers(static_cast<u32>(1));
				createInfo.setImageUsage(imageUsage);
				createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
				createInfo.setPresentMode(presentMode);
				createInfo.setOldSwapchain(oldSwapchain);
				m_swapchain = device.createSwapchainKHR(createInfo);

				if (oldSwapchain)
				{
					for (vk::ImageView& view : m_swapchainImageViews)
					{
						GetDevice()->GetDevice().destroyImageView(view);
					}
					m_swapchainImageViews.clear();
					GetDevice()->GetDevice().destroySwapchainKHR(oldSwapchain);
				}
				m_swapchainImages = GetDevice()->GetDevice().getSwapchainImagesKHR(m_swapchain);

				for (vk::Image& image : m_swapchainImages)
				{
					vk::ImageViewCreateInfo info = vk::ImageViewCreateInfo(
						{},
						image,
						vk::ImageViewType::e2D,
						PixelFormatToVulkan(m_surfaceFormat));
					info.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
					m_swapchainImageViews.push_back(GetDevice()->GetDevice().createImageView(info));
				}

				for (GPUSemaphore* semaphore : m_nextImageAvailable)
				{
					GPUSemaphoreManager::Instance().ReturnSemaphore(semaphore);
				}
				for (GPUSemaphore* semaphore : m_completedImageAvailable)
				{
					GPUSemaphoreManager::Instance().ReturnSemaphore(semaphore);
				}

				m_nextImageAvailable.resize(m_desc.ImageCount);
				m_completedImageAvailable.resize(m_desc.ImageCount);
				for (GPUSemaphore*& semaphore : m_nextImageAvailable)
				{
					semaphore = GPUSemaphoreManager::Instance().GetOrCreateSemaphore();
				}
				for (GPUSemaphore*& semaphore : m_completedImageAvailable)
				{
					semaphore = GPUSemaphoreManager::Instance().GetOrCreateSemaphore();
				}

				AcquireNextImage(nullptr, nullptr);
				//m_fence->Wait();
				//m_fence->Reset();
			}

			void GPUSwapchain_Vulkan::Destroy()
			{
				if (m_swapchain)
				{
					for (vk::ImageView& view : m_swapchainImageViews)
					{
						GetDevice()->GetDevice().destroyImageView(view);
					}
					m_swapchainImageViews.clear();
					GetDevice()->GetDevice().destroySwapchainKHR(m_swapchain);
					m_swapchain = vk::SwapchainKHR(nullptr);
				}

				if (m_surfaceKHR)
				{
					GetDevice()->GetInstance().destroySurfaceKHR(m_surfaceKHR);
					m_surfaceKHR = vk::SurfaceKHR(nullptr);
				}

				for (GPUSemaphore* semaphore : m_nextImageAvailable)
				{
					GPUSemaphoreManager::Instance().ReturnSemaphore(semaphore);
				}
				m_nextImageAvailable.clear();
				for (GPUSemaphore* semaphore : m_completedImageAvailable)
				{
					GPUSemaphoreManager::Instance().ReturnSemaphore(semaphore);
				}
				m_completedImageAvailable.clear();
			}

			void GPUSwapchain_Vulkan::Present(GPUQueue queue, u32 imageIndex, const std::vector<GPUSemaphore*>& semaphores)
			{
				const u32 semaphoreCount = static_cast<u32>(semaphores.size());
				std::vector<vk::Semaphore> semaphoresVk;
				for (size_t i = 0; i < semaphoreCount; ++i)
				{
					const GPUSemaphore_Vulkan* semaphoreVulkan = dynamic_cast<const GPUSemaphore_Vulkan*>(semaphores[i]);
					semaphoresVk.push_back(semaphoreVulkan->GetSemaphore());
				}

				// Wait for all command buffer executions to complete.
				const std::list<GPUCommandList*>&  activeCmdLists = GPUCommandListManager::Instance().GetAllInUseCommandLists(CMD_RENDERER + std::to_string(m_currentFrame));
				for (const auto& cmdList : activeCmdLists)
				{
					const GPUSemaphore_Vulkan* cmdListSemaphoreVulkan = dynamic_cast<const GPUSemaphore_Vulkan*>(cmdList->GetSignalSemaphore());
					semaphoresVk.push_back(cmdListSemaphoreVulkan->GetSemaphore());
				}

				std::array<vk::SwapchainKHR, 1> swapchains = { m_swapchain };
				std::array<u32, 1> imageIndexs = { imageIndex };
				std::array<vk::Result, 1> results;

				vk::Result result = {};
				vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR(
					semaphoresVk,
					swapchains,
					imageIndexs,
					results
				);
				vk::Result presentResult = GetDevice()->GetQueue(queue).presentKHR(presentInfo);

				m_cmdListsFromFrame[m_currentFrame] = activeCmdLists;

				m_currentFrame = (m_currentFrame + 1) % m_desc.ImageCount;

				// Wait on all the executing command lists from m_currentFrame (The previous time we submit cmd to this frame).
				for (const auto& cmdList : m_cmdListsFromFrame[m_currentFrame])
				{
					const GPUCommandList_Vulkan* cmdListVulkan = dynamic_cast<const GPUCommandList_Vulkan*>(cmdList);
					GPUFence* fence = cmdListVulkan->GetFence();
					fence->Wait();
					fence->Reset();
				}

				AcquireNextImage(nullptr, nullptr);
			}

			void GPUSwapchain_Vulkan::AcquireNextImage(GPUSemaphore* semaphore, GPUFence* fence)
			{
				const GPUSemaphore_Vulkan* semaphoreVulkan = dynamic_cast<const GPUSemaphore_Vulkan*>(m_nextImageAvailable[m_currentFrame]);
				vk::Fence waitFence = nullptr;
				if (fence)
				{
					const GPUFence_Vulkan* fenceVulkan = dynamic_cast<const GPUFence_Vulkan*>(fence);
					waitFence = fenceVulkan->GetFence();
				}
				vk::ResultValue result = GetDevice()->GetDevice().acquireNextImageKHR(m_swapchain, UINT64_MAX, semaphoreVulkan->GetSemaphore(), waitFence);
				m_nextImgeIndex = result.value;
			}

			GPUSemaphore_Vulkan* GPUSwapchain_Vulkan::GetImageAcquiredSemaphore()
			{
				GPUSemaphore_Vulkan* semaphoreVulkan = dynamic_cast<GPUSemaphore_Vulkan*>(m_nextImageAvailable[m_currentFrame]);
				return semaphoreVulkan;
			}
		}
	}
}