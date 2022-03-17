#pragma once

#include "Graphics/GPU/GPUSwapchain.h"
#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"
#include <vulkan/vulkan.hpp>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class GPUSwapchain_Vulkan : public GPUSwapchain, public GPUResource_Vulkan
			{
			public:
				virtual ~GPUSwapchain_Vulkan() override;

				virtual void Prepare() override;
				virtual void Build(GPUSwapchainDesc desc) override;
				virtual void Destroy() override;

				virtual void AcquireNextImage(GPUSemaphore* semaphore, GPUFence* fence) override;
				virtual void Present(GPUQueue queue, u32 imageIndex, const std::vector<GPUSemaphore*>& semaphores) override;

				vk::ImageView GetImageView() const { return m_swapchainImageViews[m_nextImgeIndex]; }

			private:
				vk::SwapchainKHR m_swapchain;
				vk::SurfaceKHR m_surfaceKHR;
				vk::ColorSpaceKHR m_colourSpace;
				std::vector<vk::Image> m_swapchainImages;
				std::vector<vk::ImageView> m_swapchainImageViews;
			};
		}
	}
}