#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Vulkan.h"

#include "VulkanShader.h"
#include "VulkanFramebuffer.h"
#include "Renderpass.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Semaphore.h"
#include "VulkanMaterial.h"

#include "Insight/Assimp/Mesh.h"
#include "Insight/Event/ApplicationEvent.h"

namespace Insight
{
	class Event;
}

class ImGuiRenderer;

namespace Platform
{
	class Window;

	class Fence;
	class Device;
	class VulkanMaterial;

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class IS_API Swapchain
	{
	public:
		Swapchain(const Device* device);
		~Swapchain();

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void AcquireNextImage();
		void Submit(Semaphore* waitSemaphore);
		void Draw(Semaphore* waitSemaphore, VulkanFramebuffer* offscreenFB = nullptr);
		void Present();
		Semaphore* GetAcquireNextImageSemaphore();

		void CreateSwapChain();

		const VkSwapchainKHR& GetSwapchain() const;

	private:
		void RecreateSwapchain(const Insight::Event& event);
		void DrawUI();

	private:
		Device* m_device;
		Mesh* m_fullscreenQuad;

		Insight::Render::Shader* m_swapchainShader;
		std::vector<VulkanMaterial*> m_materials;
		CommandPool* m_drawCommandPool;
		std::vector<CommandBuffer*> m_drawCommandBuffers;
		std::vector<Fence*> m_inFlightFences;
		std::vector<Fence*> m_imagesInFlight;

		const uint32_t MaxFramesInFlight = 3;

		int tempShader = 0;
		int shaderIndex = 0;

#ifdef IS_EDITOR
		CommandBuffer* m_editorCommandBuffer;
		CommandPool* m_editorCommandPool;
		VulkanFramebuffer* m_editorFrameBuffer;

		void* m_sceneTexture = nullptr;
#endif
		uint32_t m_currentFrame = 0;
		uint32_t m_imageIndex;
		SwapChainSupportDetails m_swapChainDetails;
		VkSwapchainKHR m_swapchain;
		std::vector<VulkanFramebuffer*> m_swapchainFramebuffers;

		friend ImGuiRenderer;
	};
}
#endif