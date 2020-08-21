#pragma once

#ifdef IMGUI_ENABLED
#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Event/ApplicationEvent.h"
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"

#ifdef IS_VULKAN
#include "examples/imgui_impl_vulkan.h"
#endif // IS_VULKAN


namespace Insight
{
	class Renderer;

	namespace Render
	{
		class VulkanFramebuffer;
		class VulkanRenderer;
		class Shader;
		class VulkanMaterial;
		class CommandPool;
		class CommandBuffer;

		class ImGuiRenderer : public TSingleton<ImGuiRenderer>
		{
		public:
			ImGuiRenderer(Renderer* renderer);
			~ImGuiRenderer();

			void NewFrame();
			void EndFrame();
			void Render(CommandBuffer* commandBuffer);

		private:
			void WindowResize(const Event& event);

#ifdef IS_VULKAN
			void CreateDescPool();

			VkDescriptorPool m_descPool;
			VulkanRenderer* m_vulkanRenderer;
			CommandPool* m_commandPool;
			CommandBuffer* m_commandBuffer;
			VulkanFramebuffer* m_framebuffer;

			VkImage m_fontImage;
			VkImageView m_fontImageView;
			VkSampler m_fontSampler;
			VkDeviceMemory m_fontMemory;

			VkDeviceSize m_bufferMemoryAlignment = 256;
			VkDeviceMemory m_uploadBufferMemory = VK_NULL_HANDLE;
			VkBuffer m_uploadBuffer = VK_NULL_HANDLE;
#endif
		};
	}
}
#endif