#pragma once

#include "Engine/Graphics/GPUSwapchain.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUSwapchainVulkan : public Graphics::GPUSwapchain
	{
	public:
		GPUSwapchainVulkan();
		virtual ~GPUSwapchainVulkan() override;

		virtual void Init() override;
		virtual void Build(const Graphics::GPUSwapchainDesc& desc) override;

		virtual GPUResults GetNextImage(Graphics::GPUSemaphore* presentCompleted, u32* imageIndex) override;
		virtual GPUResults Present(GPUQueue queue, u32 imageIndex, std::vector<Graphics::GPUSemaphore*>  waitSemaphores = {}) override;
		virtual void ReleaseGPU() override;
		virtual u32 GetImageCount() override;

		VkFormat GetColorFormat() const { return m_colorFormat; }

	private:
		GPUDeviceVulkan* m_device;
		VkFormat m_colorFormat;
		VkColorSpaceKHR m_colorSpace;
		VkSwapchainKHR m_swapchain;
		VkSurfaceKHR m_surface;
		u32 m_graphicsNodeQueueIndex;
		u32 m_presentNodeQueueIndex;
	};
}