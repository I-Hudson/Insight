#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/Lowlevel/Renderpass.h"
#include "Insight/Renderer/Lowlevel/CommandBuffer.h"
#include "Insight/Renderer/Lowlevel/Semaphore.h"
#include "Insight/Renderer/Lowlevel/Fence.h"

namespace Insight
{
	namespace Render
	{
		class Device;

		class IS_API Framebuffer
		{
		public:
			Framebuffer() = default;
			Framebuffer(const Device* device, VkImage* image, VkFormat format, VkExtent2D extent, Renderpass* renderpass);
			~Framebuffer();

			void BindBuffer(CommandBuffer* commandBuffers, const VkSubpassContents& subpassContents = VK_SUBPASS_CONTENTS_INLINE);
			void SetRenderPass(Renderpass* renderpass);

			Semaphore* GetAvailbleSem() const { return m_imageAvailableSem; }
			Semaphore* GetFinishedSem() const { return m_imageFinishedSem; }
			Fence* GetFence() const { return m_fence; }

		private:
			const Device* m_device;

			VkImage m_image;
			VkImageView m_view;
			VkDeviceMemory m_mem;
			VkFormat m_format;
			VkExtent2D m_extent;
			VkFramebuffer m_frameBuffer;
			Render::Renderpass* m_renderpass;

			Semaphore* m_imageAvailableSem;
			Semaphore* m_imageFinishedSem;
			Fence* m_fence;
		};
	}
}


