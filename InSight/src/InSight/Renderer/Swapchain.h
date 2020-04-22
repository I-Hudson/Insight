#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"

namespace Insight
{
	class Window;

	namespace Render
	{
		class Device;

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};
		struct VulkanImage
		{
			VkImage Image;
			VkImageView View;
			VkDeviceMemory Mem;
			VkFormat Format;
			VkExtent2D Extent;
		};

		struct SwapchainSettings
		{
			const Window* Window;			
			const Device* Device;
		};

		class IS_API Swapchain
		{
		public: 
			Swapchain(const SwapchainSettings swapchainSettings);
			~Swapchain();

			SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

			VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
			VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
			VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			void CreateSwapChain();
			void CreateSwapChainImageViews();

			const VkSwapchainKHR& GetSwapchain() const;

		private:
			SwapchainSettings m_swapchainSettings;

			SwapChainSupportDetails m_swapChainDetails;
			VkSwapchainKHR m_swapchain;
			std::vector<VulkanImage> m_swapChainImages;
		};
	}
}