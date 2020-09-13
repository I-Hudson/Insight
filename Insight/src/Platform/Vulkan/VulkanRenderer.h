#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Insight/Renderer/Renderer.h"
#include "Insight/Module/WindowModule.h"
#include "Insight/Renderer/ShaderModule.h"

#include "Device.h"
#include "Queue.h"
#include "Swapchain.h"
#include "VulkanMaterial.h"
#include "Insight/Renderer/ImGuiRenderer.h"

class CameraComponent;

namespace Insight
{
	class Event;
}

namespace Platform
{
	class VulkanMaterial;

	class IS_API VulkanRenderer : public Insight::Renderer
	{
	public:
		VulkanRenderer();
		virtual ~VulkanRenderer() override;

		virtual void Clear() override;
		virtual void Render(CameraComponent* mainCamera, std::vector<MeshComponent*> meshes) override;
		virtual void Present() override;

		virtual Material* GetDefaultMaterial() override;

		const VkInstance& GetInstance() { return m_device->GetInstance(); }
		const VkDevice& GetDevice() { return m_device->GetDevice(); }
		const VkPhysicalDevice& GetPhysicalDevice() const { return m_device->GetPhysicalDevice(); }
		const VkSurfaceKHR& GetSurface() const { return m_surface; }
		Device* GetDeviceWrapper() const { return m_device; }

		Queue GetGraphicsQueue() const { return m_device->GetQueue(QueueFamilyType::Graphics); }
		Queue GetPresentQueue() const { return m_device->GetQueue(QueueFamilyType::Present); }

	private:

		void RecreateFramebuffers(const Insight::Event& event);
		void DeserializeFromFile(const Insight::Event& event);

		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

		bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);
		std::vector<const char*> GetRequiredExtensions();
		void VulkanEnumExtProps();

	private:

		ImGuiRenderer* m_imguiRenderer;
		bool m_recordCommandBuffers;

		// Vulkan objects
		Device* m_device;
		Swapchain* m_swapchain;

		Fence* m_commndFence;
		CommandPool* m_commandPool;
		CommandBuffer* m_commandBuffer;
		Insight::Render::Shader* m_shader;
		VulkanMaterial* m_material;
		VulkanFramebuffer* m_framebuffer;

		VkSurfaceKHR m_surface;

		uint64_t vertexCount = 0;
		uint64_t triCount = 0;
		uint64_t meshCount = 0;

		bool m_enableValidationLayers = true;
		std::vector<const char*> m_validationLayers;

		std::vector<const char*> m_deviceExtensions;

		Insight::Module::WindowModule* m_windowModule;

		friend VulkanMaterial;
	};
}
#endif