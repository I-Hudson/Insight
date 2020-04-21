#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/Renderer.h"
#include "Insight/Module/WindowModule.h"

#include <set>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Insight
{
	namespace Render
	{
		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool IsComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

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

		class IS_API VulkanRenderer : public Renderer
		{
		public:
			VulkanRenderer(RendererStartUpData& startupData);
			virtual ~VulkanRenderer() override;

			virtual void Clear() override;
			virtual void Render() override;
			virtual void Present() override;

			const VkPhysicalDevice& GetPhysDevice() const { return m_physicalDevice; }
			const VkDevice& GetDevice() const { return m_device; }

			const VkSurfaceKHR& GetSurface() const { return m_surface; }
			//const VkSurfaceFormatKHR& GetSurfaceFormat() const { return m_surfaceFormat; }
			//const VkSurfaceCapabilitiesKHR GetSurfaceCaps() const { return m_surfaceCaps; }

			VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
			VkQueue GetPresentQueue() const { return m_presentQueue; }

			VkInstance& GetInstance() { return m_inst; }
			VkDevice& GetDevice() { return m_device; }

		private:
			void CreateSurface();
			void SelectPhysicalDevice();
			void CreateLogicalDevice();

			bool IsDeviceSuitable(VkPhysicalDevice device);
			bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

			QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
			SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

			VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
			VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
			VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			void CreateSwapChain();
			void CreateSwapChainImageViews();

			bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);
			std::vector<const char*> GetRequiredExtensions();
			void VulkanEnumExtProps();


			void SetupDebugMessenger();
			void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		private:

			// Vulkan objects
			VkInstance m_inst;
			VkDevice m_device;
			VkPhysicalDevice m_physicalDevice;

			VkSurfaceKHR m_surface;
			SwapChainSupportDetails m_swapChainDetails;
			VkSwapchainKHR m_swapChain;
			std::vector<VulkanImage> m_swapChainImages;

			VkDebugUtilsMessengerEXT m_debugMessenger;

			bool m_enableValidationLayers = true;
			std::vector<const char*> m_validationLayers;

			std::vector<const char*> m_deviceExtensions;

			// Internal stuff
			VkQueue m_graphicsQueue;
			VkQueue m_presentQueue;

			Module::WindowModule* m_windowModule;
		};
	}
}

