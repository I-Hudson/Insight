#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/Shader.h"
#include "Insight/Renderer/Lowlevel/Framebuffer.h"
#include "Insight/Renderer/Lowlevel/Renderpass.h"
#include "Insight/Renderer/Lowlevel/CommandPool.h"
#include "Insight/Renderer/Lowlevel/CommandBuffer.h"
#include "Insight/Renderer/Lowlevel/Semaphore.h"

#include "Insight/Event/ApplicationEvent.h"

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

		struct SwapchainSettings
		{
			const Window* Window;			
			Device* Device;
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
			
			void AcquireNextImage();
			void Submit(Semaphore* waitSemaphore);
			void Draw(Semaphore* waitSemaphore);
			void Present();
			Semaphore* GetAcquireNextImageSemaphore();

			void CreateSwapChain();

			const VkSwapchainKHR& GetSwapchain() const;

		private:
			void RecreateSwapchain(const Event& event);

		private:
			SwapchainSettings m_swapchainSettings;

			Shader* m_swapchainShader;
			CommandPool* m_drawCommandPool;
			std::vector<CommandBuffer*> m_drawCommandBuffers;
			std::vector<Fence*> m_inFlightFences;
			std::vector<Fence*> m_imagesInFlight;

			const uint32_t MaxFramesInFlight = 3;

			int tempShader = 0;
			int shaderIndex = 0;

			uint32_t m_currentFrame = 0;
			uint32_t m_imageIndex;
			SwapChainSupportDetails m_swapChainDetails;
			VkSwapchainKHR m_swapchain;
			std::vector<Framebuffer*> m_swapchainFramebuffers;
		};
	}
}