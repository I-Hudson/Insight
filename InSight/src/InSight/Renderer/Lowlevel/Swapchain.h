#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/Shader.h"
#include "Insight/Renderer/Lowlevel/Framebuffer.h"
#include "Insight/Renderer/Lowlevel/Renderpass.h"
#include "Insight/Renderer/Lowlevel/CommandPool.h"
#include "Insight/Renderer/Lowlevel/CommandBuffer.h"
#include "Insight/Renderer/Lowlevel/Semaphore.h"

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
			void Draw();
			void Present();

			void CreateSwapChain();

			const VkSwapchainKHR& GetSwapchain() const;

		private:
			SwapchainSettings m_swapchainSettings;

			Shader* m_swapchainShader;
			Shader* m_swapchainTestShader;
			CommandPool* m_drawCommandPool;
			std::vector<CommandBuffer*> m_drawCommandBuffers;

			const uint32_t MaxFramesInFlight = 3;

			int tempShader = 0;
			int shaderIndex = 0;

			uint32_t m_currentFrame = 0;
			uint32_t m_imageIndex;
			Semaphore* m_imageAvaliable;
			Semaphore* m_renderFinished;
			Renderpass* m_renderpass;
			SwapChainSupportDetails m_swapChainDetails;
			VkSwapchainKHR m_swapchain;
			std::vector<Framebuffer*> m_swapchainFramebuffers;
		};
	}
}