#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUSwapchainVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUAdapterVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUImageVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/GPUSyncVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/Module/WindowModule.h"
#include <GLFW/glfw3.h>

namespace Insight::GraphicsAPI::Vulkan
{
	GPUSwapchainVulkan::GPUSwapchainVulkan()
		: m_swapchain(VK_NULL_HANDLE)
	{ }

	GPUSwapchainVulkan::~GPUSwapchainVulkan()
	{
		ReleaseGPU();
	}

	void GPUSwapchainVulkan::Init()
	{
		m_device = static_cast<GPUDeviceVulkan*>(GPUDevice::Instance());

		ASSERT(glfwCreateWindowSurface(m_device->m_instance, Window::m_window, nullptr, &m_surface) == VK_SUCCESS &&
			   "[GPUSwapchainVulkan::Init] Could not create window surface.");

		GPUAdapterVulkan* gpuAdapter = static_cast<GPUAdapterVulkan*>(m_device->GetAdapter());

		// Get available queue family properties
		uint32_t queueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(gpuAdapter->Gpu, &queueCount, NULL);
		assert(queueCount >= 1);

		std::vector<VkQueueFamilyProperties> queueProps(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(gpuAdapter->Gpu, &queueCount, queueProps.data());

		// Iterate over each queue to learn whether it supports presenting:
		// Find a queue with present support
		// Will be used to present the swap chain images to the windowing system
		std::vector<VkBool32> supportsPresent(queueCount);
		for (uint32_t i = 0; i < queueCount; i++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(gpuAdapter->Gpu, i, m_surface, &supportsPresent[i]);
		}

		// Search for a graphics and a present queue in the array of queue
		// families, try to find one that supports both
		uint32_t graphicsQueueNodeIndex = U32_MAX;
		uint32_t presentQueueNodeIndex = U32_MAX;
		for (uint32_t i = 0; i < queueCount; i++)
		{
			if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
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
		if (presentQueueNodeIndex == U32_MAX)
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
		if (graphicsQueueNodeIndex == U32_MAX || presentQueueNodeIndex == U32_MAX)
		{
			ASSERT(false && "[GPUSwapchainVulkan::Init] Could not find a graphics and/or presenting queue!");
		}

		// todo : Add support for separate graphics and presenting queue
		if (graphicsQueueNodeIndex != presentQueueNodeIndex)
		{
			ASSERT(false && "[GPUSwapchainVulkan::Init] Separate graphics and presenting queues are not supported yet!");
		}
		m_graphicsNodeQueueIndex = graphicsQueueNodeIndex;
		m_presentNodeQueueIndex = presentQueueNodeIndex;

		// Get list of supported surface formats
		uint32_t formatCount;
		ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(gpuAdapter->Gpu, m_surface, &formatCount, NULL));
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(gpuAdapter->Gpu, m_surface, &formatCount, surfaceFormats.data()));

		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			m_colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
			m_colorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			// iterate over the list of available surface format and
			// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
			bool found_B8G8R8A8_UNORM = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					m_colorFormat = surfaceFormat.format;
					m_colorSpace = surfaceFormat.colorSpace;
					found_B8G8R8A8_UNORM = true;
					break;
				}
			}

			// in case VK_FORMAT_B8G8R8A8_UNORM is not available
			// select the first available color format
			if (!found_B8G8R8A8_UNORM)
			{
				m_colorFormat = surfaceFormats[0].format;
				m_colorSpace = surfaceFormats[0].colorSpace;
			}
		}
	}

	void GPUSwapchainVulkan::Build(const Graphics::GPUSwapchainDesc& desc)
	{
		m_desc = desc;
		VkSwapchainKHR oldSwapchain = m_swapchain;
		GPUAdapterVulkan* gpuAdapter = static_cast<GPUAdapterVulkan*>(m_device->GetAdapter());

		// Get physical device surface properties and formats
		VkSurfaceCapabilitiesKHR surfCaps;
		ThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpuAdapter->Gpu, m_surface, &surfCaps));

		// Get available present modes
		uint32_t presentModeCount;
		ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(gpuAdapter->Gpu, m_surface, &presentModeCount, NULL));
		IS_CORE_ASSERT(presentModeCount > 0, "There are no present modes.");

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(gpuAdapter->Gpu, m_surface, &presentModeCount, presentModes.data()));

		VkExtent2D swapchainExtent = {};
		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (surfCaps.currentExtent.width == (u32)-1)
		{
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = m_desc.Width;
			swapchainExtent.height = m_desc.Height;
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = surfCaps.currentExtent;
			m_desc.Width = surfCaps.currentExtent.width;
			m_desc.Height = surfCaps.currentExtent.height;
		}

		// Select a present mode for the swapchain

		// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// This mode waits for the vertical blank ("v-sync")
		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		// If v-sync is not requested, try to find a mailbox mode
		// It's the lowest latency non-tearing present mode available
		if (!m_desc.VSync)
		{
			for (size_t i = 0; i < presentModeCount; i++)
			{
				if (m_desc.GSync)
				{
					if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
					{
						swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
						break;
					}
				}

				if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
				{
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}

		// Determine the number of images
		uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
		if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
		{
			desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
		}

		// Find the transformation of the surface
		VkSurfaceTransformFlagsKHR preTransform;
		if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			// We prefer a non-rotated transform
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			preTransform = surfCaps.currentTransform;
		}

		// Find a supported composite alpha format (not all devices support alpha opaque)
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Simply select the first composite alpha format available
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags)
		{
			if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag)
			{
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		VkSwapchainCreateInfoKHR swapchainCI = { };
		swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCI.pNext = NULL;
		swapchainCI.surface = m_surface;
		swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
		swapchainCI.imageFormat = m_colorFormat;
		swapchainCI.imageColorSpace = m_colorSpace;
		swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCI.queueFamilyIndexCount = 0;
		swapchainCI.pQueueFamilyIndices = NULL;
		swapchainCI.presentMode = swapchainPresentMode;
		swapchainCI.oldSwapchain = oldSwapchain;
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		swapchainCI.clipped = VK_TRUE;
		swapchainCI.compositeAlpha = compositeAlpha;

		// Enable transfer source on swap chain images if supported
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		{
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		// Enable transfer destination on swap chain images if supported
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		{
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		ThrowIfFailed(vkCreateSwapchainKHR(m_device->Device, &swapchainCI, nullptr, &m_swapchain));

		// If an existing swap chain is re-created, destroy the old swap chain
		// This also cleans up all the presentable images
		if (oldSwapchain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < m_swapchainImageCount; ++i)
			{
				m_swapchainImages[i].View->ReleaseGPU();
			}
			vkDestroySwapchainKHR(m_device->Device, oldSwapchain, nullptr);
		}
		ThrowIfFailed(vkGetSwapchainImagesKHR(m_device->Device, m_swapchain, &m_swapchainImageCount, NULL));

		// Get the swap chain images
		std::vector<VkImage> images;
		images.resize(m_swapchainImageCount);
		ThrowIfFailed(vkGetSwapchainImagesKHR(m_device->Device, m_swapchain, &m_swapchainImageCount, images.data()));

		if (m_swapchain != VK_NULL_HANDLE)
		{
			for (auto& image : m_swapchainImages)
			{
				::Delete(image.Image);
				image.Image = nullptr;
				::Delete(image.View);
				image.View = nullptr;
			}
		}

		m_swapchainImages.resize(m_swapchainImageCount);
		for (u32 i = 0; i < m_swapchainImageCount; ++i)
		{
			if (m_swapchainImages[i].Image == nullptr)
			{
				m_swapchainImages[i].Image = Graphics::GPUImage::New();
				m_swapchainImages[i].View = Graphics::GPUImageView::New();
			}
			m_swapchainImages[i].Image->Init(Graphics::GPUImageDesc::SwapchainImage(m_desc.Width, m_desc.Height, FromVulkanFormat(m_colorFormat), images[i]));
			m_swapchainImages[i].Image->SetName("Swapchain Image" + std::to_string(i));
			m_swapchainImages[i].View->Init(m_swapchainImages[i].Image);
			m_swapchainImages[i].View->SetName("Swapchain Image View" + std::to_string(i));
		}
	}

	GPUResults GPUSwapchainVulkan::GetNextImage(Graphics::GPUSemaphore* presentCompleted, u32* imageIndex)
	{
		VkSemaphore semaphore = static_cast<GPUSemaphoreVulkan*>(presentCompleted)->m_semaphoreVulkan;
		return static_cast<GPUResults>(vkAcquireNextImageKHR(m_device->Device, m_swapchain, U64_MAX, semaphore, nullptr, imageIndex));
	}

	GPUResults GPUSwapchainVulkan::Present(GPUQueue queue, u32 imageIndex, std::vector<Graphics::GPUSemaphore*>  waitSemaphores)
	{
		std::vector<VkSemaphore> waitSemaphoresVulkan;
		waitSemaphoresVulkan.reserve(waitSemaphores.size());
		for (auto* semaphore : waitSemaphores)
		{
			waitSemaphoresVulkan.push_back(static_cast<GPUSemaphoreVulkan*>(semaphore)->m_semaphoreVulkan);
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &imageIndex;
		// Check if a wait semaphore has been specified to wait for before presenting the image
		presentInfo.pWaitSemaphores = waitSemaphoresVulkan.data();
		presentInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphoresVulkan.size());
		IS_PROFILE_GPU_FLIP(&m_swapchain);
		return static_cast<GPUResults>(vkQueuePresentKHR(m_device->GetQueue(queue), &presentInfo));
	}

	void GPUSwapchainVulkan::ReleaseGPU()
	{
		for (auto& image : m_swapchainImages)
		{
			image.Image->ReleaseGPU();
			image.View->ReleaseGPU();
			::Delete(image.Image);
			::Delete(image.View);
		}
		m_swapchainImages.clear();

		if (m_surface)
		{
			vkDestroySwapchainKHR(m_device->Device, m_swapchain, nullptr);
			m_swapchain = VK_NULL_HANDLE;
			vkDestroySurfaceKHR(m_device->m_instance, m_surface, nullptr);
			m_surface = VK_NULL_HANDLE;
		}
	}

	u32 GPUSwapchainVulkan::GetImageCount()
	{
		return static_cast<u32>(m_swapchainImages.size());
	}
}
