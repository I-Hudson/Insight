#include "ispch.h"
#include "Swapchain.h"

#include "Insight/Renderer/Device.h"
#include "Insight/Module/WindowModule.h"
#include "Insight/Renderer/VulkanInits.h"

namespace Insight
{
	namespace Render
	{
		Swapchain::Swapchain(const SwapchainSettings swapchainSettings)
			: m_swapchainSettings(swapchainSettings)
		{
			CreateSwapChain();
			CreateSwapChainImageViews();
		}

		Swapchain::~Swapchain()
		{
			for (auto swapChainImage : m_swapChainImages)
			{
				vkDestroyImageView(m_swapchainSettings.Device->GetDevice(), swapChainImage.View, nullptr);
			}
			vkDestroySwapchainKHR(m_swapchainSettings.Device->GetDevice(), m_swapchain, nullptr);
		}

		SwapChainSupportDetails Swapchain::QuerySwapChainSupport(VkPhysicalDevice device)
		{
			SwapChainSupportDetails details;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_swapchainSettings.Device->GetSurface(), &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_swapchainSettings.Device->GetSurface(), &formatCount, nullptr);
			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_swapchainSettings.Device->GetSurface(), &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_swapchainSettings.Device->GetSurface(), &presentModeCount, nullptr);
			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_swapchainSettings.Device->GetSurface(), &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
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

		VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
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

		VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
		{
			if (capabilities.currentExtent.width != UINT32_MAX)
			{
				return capabilities.currentExtent;
			}
			else
			{
				uint32_t width = static_cast<uint32_t>(m_swapchainSettings.Window->GetWidth());
				uint32_t height = static_cast<uint32_t>(m_swapchainSettings.Window->GetHeight());
				VkExtent2D actualExtent = { width, height };

				actualExtent.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}

		void Swapchain::CreateSwapChain()
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_swapchainSettings.Device->GetPhysicalDevice());

			VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
			{
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo = VulkanInits::SwapChainInfo();
			createInfo.surface = m_swapchainSettings.Device->GetSurface();
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			std::vector<uint32_t> queueFamilyIndices
			{
				m_swapchainSettings.Device->GetQueueFamily(Render::QueueFamilyType::Graphics).GetValue(),
				m_swapchainSettings.Device->GetQueueFamily(Render::QueueFamilyType::Present).GetValue()
			};

			if (m_swapchainSettings.Device->GetQueueFamily(Render::QueueFamilyType::Graphics).GetValue() !=
				m_swapchainSettings.Device->GetQueueFamily(Render::QueueFamilyType::Present).GetValue())
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
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

			ThrowIfFailed(vkCreateSwapchainKHR(m_swapchainSettings.Device->GetDevice(), &createInfo, nullptr, &m_swapchain));

			imageCount = 0;
			std::vector<VkImage> swapChainImages;
			vkGetSwapchainImagesKHR(m_swapchainSettings.Device->GetDevice(), m_swapchain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(m_swapchainSettings.Device->GetDevice(), m_swapchain, &imageCount, swapChainImages.data());

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

		void Swapchain::CreateSwapChainImageViews()
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

				ThrowIfFailed(vkCreateImageView(m_swapchainSettings.Device->GetDevice(), &createInfo, nullptr, &m_swapChainImages[i].View));
			}

			IS_CORE_INFO("SwapChain image view completed.");
		}

		const VkSwapchainKHR& Swapchain::GetSwapchain() const
		{
			return m_swapchain;
		}
	}
}