#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Vulkan.h"
#include "Insight/Renderer/Framebuffer.h"
#include "Renderpass.h"
#include "CommandBuffer.h"
#include "Semaphore.h"
#include "Fence.h"

namespace Platform
{
	class Device;

	struct FrameBufferAttachment
	{
		VkImage Image;
		VkImageView View;
		VkDeviceMemory Mem;
		VkFormat Format;
		VkImageUsageFlags ImageUsage;
		VkImageAspectFlags ImageViewAspect;
		VkImageLayout ImageLayout;
		VkImageLayout FinalLayout;
		bool DeleteImage;
	};

	class IS_API VulkanFramebuffer : public Insight::Render::Framebuffer
	{
	public:
		VulkanFramebuffer(Device* device, const int& width, const int& height);
		VulkanFramebuffer(Device* device, VkExtent2D extent);
		VulkanFramebuffer(Device* device, VkImage* image, VkFormat format, VkExtent2D extent);
		~VulkanFramebuffer();

		virtual void CreateAttachment(const uint32_t& format, const uint32_t& usage, const uint32_t& imageLayout, const uint32_t& finalLayout) override;
		virtual void CompileFrameBuffer() override;
		virtual FrameBufferAttachment& GetAttachment(const int& index) override;

		void AttachImage(VkImage* image, VkFormat format, const VkImageLayout& imageLayout, const VkImageLayout& finalLayout);

		void BindBuffer(CommandBuffer* commandBuffers, const VkSubpassContents& subpassContents = VK_SUBPASS_CONTENTS_INLINE);
		void UnbindBuffer(CommandBuffer* commandBuffers);
		void SetRenderPass(Renderpass* renderpass);

		void Resize(int width, int height);

		Renderpass* GetRenderpass() const { return m_renderpass; }
		VkSampler* GetSampler() { return &m_sampler; }
		Semaphore* GetAvailbleSem() const { return m_imageAvailableSem; }
		Semaphore* GetFinishedSem() const { return m_imageFinishedSem; }
		Fence* GetFence() const { return m_fence; }

	private:
		void CreateImage(FrameBufferAttachment& attachment);
		void CreateImageView(FrameBufferAttachment& attachment);
		void CreateMemory(FrameBufferAttachment& attachment);
		void CreateSampler();

	private:
		Device* m_device;

		std::vector<FrameBufferAttachment> m_attachments;
		VkExtent2D m_extent;
		VkFramebuffer m_frameBuffer;
		VkSampler m_sampler;
		Renderpass* m_renderpass;

		CommandBuffer* m_commandBuffer;
		Semaphore* m_imageAvailableSem;
		Semaphore* m_imageFinishedSem;
		Fence* m_fence;
	};
}
#endif