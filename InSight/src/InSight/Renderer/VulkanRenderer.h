#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/Renderer.h"
#include "Insight/Module/WindowModule.h"

#include "Insight/Renderer/Lowlevel/Device.h"
#include "Insight/Renderer/Lowlevel/Swapchain.h"
#include "Insight/Renderer/Lowlevel/ShaderModule.h"

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
			virtual void Render(std::vector<MeshComponent*> meshes) override;
			virtual void Present() override;


			const VkInstance& GetInstance() { return m_device->GetInstance(); }
			const VkDevice& GetDevice() { return m_device->GetDevice(); }
			const VkPhysicalDevice& GetPhysicalDevice() const { return m_device->GetPhysicalDevice(); }
			const VkSurfaceKHR& GetSurface() const { return m_surface; }
			Device* GetDeviceWrapper() const { return m_device; }

			Queue GetGraphicsQueue() const { return m_device->GetQueue(QueueFamilyType::Graphics); }
			Queue GetPresentQueue() const { return m_device->GetQueue(QueueFamilyType::Present); }


		private:

			void RecreateFramebuffers(const Event& event);

			bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

			bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);
			std::vector<const char*> GetRequiredExtensions();
			void VulkanEnumExtProps();

		private:

			// Vulkan objects
			Device* m_device;
			Swapchain* m_swapchain;

			Fence* m_commndFence;
			CommandPool* m_commandPool;
			CommandBuffer* m_commandBuffer;
			Shader* m_shader;
			Framebuffer* m_framebuffer;

			VkSurfaceKHR m_surface;

			bool m_enableValidationLayers = true;
			std::vector<const char*> m_validationLayers;

			std::vector<const char*> m_deviceExtensions;

			Module::WindowModule* m_windowModule;
		};
	}
}

