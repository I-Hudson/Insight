#pragma once
#ifdef IS_VULKAN
#include "Insight/Core/Core.h"
#include "VulkanHeader.h"

#include "Insight/Event/ApplicationEvent.h"
#include "Insight/Renderer/FrameGraph/FrameGraph.h"

namespace Insight
{
	class Event;
	class ImGuiRenderer;
}

namespace vks
{
	typedef struct _SwapChainBuffers
	{
		VkImage image;
		VkImageView view;
	} SwapChainBuffer;

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class IS_API Swapchain : public Insight::Object
	{
	public:

		void Create(uint32_t width, uint32_t height, bool vsync = false, bool gsync = false);
		void InitSurface(void* platformWindow);
		void Connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
		VkResult AcquireNextImage(VkSemaphore presentCompleteSem, uint32_t* imageIndex);
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
		void CleanUp();

		uint32_t GetImageCount() { return m_imageCount; }
		uint32_t GetQueuNodeIndex() { return m_queueNodeIndex; }
		VkImageView GetImageView(uint32_t index) { return m_buffers[index].view; }
		VkFormat GetColorFormat() { return m_colorFormat; }

	private:
		VkFormat m_colorFormat;
		VkColorSpaceKHR m_colorSpace;
		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		uint32_t m_imageCount;
		std::vector<VkImage> m_images;
		std::vector<SwapChainBuffer> m_buffers;
		uint32_t m_queueNodeIndex = UINT32_MAX;

		VkInstance m_instance;
		VkDevice m_device;
		VkPhysicalDevice m_physicalDevice;
		VkSurfaceKHR m_surface;
		// Function pointers
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
		PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
		PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
		PFN_vkQueuePresentKHR fpQueuePresentKHR;
	};
}
#endif