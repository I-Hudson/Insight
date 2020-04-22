#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/Renderer.h"
#include "Insight/Module/WindowModule.h"

#include "Device.h"
#include "Swapchain.h"

#include "Vulkan.h"

#include <set>
#include <optional>

namespace Insight
{
	namespace Render
	{
		
		class IS_API VulkanRenderer : public Renderer
		{
		public:
			VulkanRenderer(RendererStartUpData& startupData);
			virtual ~VulkanRenderer() override;

			virtual void Clear() override;
			virtual void Render() override;
			virtual void Present() override;


			const VkInstance& GetInstance() { return m_device->GetInstance(); }
			const VkDevice& GetDevice() { return m_device->GetDevice(); }
			const VkPhysicalDevice& GetPhysicalDevice() const { return m_device->GetPhysicalDevice(); }

			const VkSurfaceKHR& GetSurface() const { return m_surface; }
			//const VkSurfaceFormatKHR& GetSurfaceFormat() const { return m_surfaceFormat; }
			//const VkSurfaceCapabilitiesKHR GetSurfaceCaps() const { return m_surfaceCaps; }

			Queue GetGraphicsQueue() const { return m_device->GetQueue(QueueFamilyType::Graphics); }
			Queue GetPresentQueue() const { return m_device->GetQueue(QueueFamilyType::Present); }


		private:

			bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

			bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);
			std::vector<const char*> GetRequiredExtensions();
			void VulkanEnumExtProps();

		private:

			// Vulkan objects
			Device* m_device;
			Swapchain* m_swapchain;

			VkSurfaceKHR m_surface;

			bool m_enableValidationLayers = true;
			std::vector<const char*> m_validationLayers;

			std::vector<const char*> m_deviceExtensions;

			Module::WindowModule* m_windowModule;
		};
	}
}

