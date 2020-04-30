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

		struct FrameBufferAttachment
		{
			VkImage Image;
			VkImageView View;
			VkDeviceMemory Mem;
			VkFormat Format;
			VkImageLayout ImageLayout;
			VkImageLayout FinalLayout;
			bool DeleteImage;
		};

		class IS_API Framebuffer
		{
		public:
			Framebuffer(Device* device, const int& width, const int& height);
			Framebuffer(Device* device, VkExtent2D extent);
			Framebuffer(Device* device, VkImage* image, VkFormat format, VkExtent2D extent);
			~Framebuffer();

			void CreateAttachment(VkFormat format, VkImageUsageFlags usage, const VkImageLayout& imageLayout, const VkImageLayout& finalLayout);
			void CompileFrameBuffer();

			void AttachImage(VkImage* image, VkFormat format, const VkImageLayout& imageLayout, const VkImageLayout& finalLayout);

			void BindBuffer(CommandBuffer* commandBuffers, const VkSubpassContents& subpassContents = VK_SUBPASS_CONTENTS_INLINE);
			void UnbindBuffer(CommandBuffer* commandBuffers);
			void SetRenderPass(Renderpass* renderpass);

			void Resize(int width, int height);

			Renderpass* GetRenderpass() const { return m_renderpass; }
			Semaphore* GetAvailbleSem() const { return m_imageAvailableSem; }
			Semaphore* GetFinishedSem() const { return m_imageFinishedSem; }
			Fence* GetFence() const { return m_fence; }

		private:
			Device* m_device;

			std::vector<FrameBufferAttachment> m_attachments;
			VkExtent2D m_extent;
			VkFramebuffer m_frameBuffer;
			Render::Renderpass* m_renderpass;

			CommandBuffer* m_commandBuffer;
			Semaphore* m_imageAvailableSem;
			Semaphore* m_imageFinishedSem;
			Fence* m_fence;
		};
	}
}


