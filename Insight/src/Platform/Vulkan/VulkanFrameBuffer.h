#pragma once
#include "Insight/Core/Core.h"
#include "Platform/Vulkan/VulkanHeader.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace vks
{
	struct FrameBufferAttachment
	{
		std::string Name;
		VkImage Image;
		VkImageView ImageView;
		VkDeviceMemory DeviceMemory;
		VkImageLayout ImageLayout;
		VkFormat Format;
	};

	struct RenderPassInfo
	{
		U32 ColorAttachmentCount;
	};

	class VulkanFrameBuffer
	{
	public:
		VulkanFrameBuffer();
		~VulkanFrameBuffer();

		void SetRect(const U32& width, const U32& height);
		void CreateAttachment(const U32& format, const U32& imageUsage, const std::string& attachmentName);
		void CreateRenderPass();

		const FrameBufferAttachment& GetAttachment(const std::string& attachmentName);
		std::vector<VkClearValue> GetClearAttachments();
		void WaitForFence();
		void ResetFence();

		const U32& GetWidth() const { return m_width; }
		const U32& GetHeight() const { return m_height; }
		VkFramebuffer GetFrameBuffer() const { return m_frameBuffer; }
		VkRenderPass GetRenderPass() const { return m_renderPass; }
		const RenderPassInfo& GetRenderPassInfo() const { return m_renderPassInfo; }
		VkSampler GetSampler() const { return m_sampler; }

		VkSemaphore GetReadySemaphore() const { return m_readySemaphore; }
		VkSemaphore GetFinishedSemaphore() const { return m_finishedSemaphore; }
		VkFence GetFence() const { return m_fence; }

	private:
		U32 m_width;
		U32 m_height;

		VkFramebuffer m_frameBuffer;
		std::vector<FrameBufferAttachment> m_attachmnets;
		VkRenderPass m_renderPass;
		RenderPassInfo m_renderPassInfo;
		VkSampler m_sampler;

		VkSemaphore m_readySemaphore;
		VkSemaphore m_finishedSemaphore;
		VkFence m_fence;
		bool m_destroyed;
	};
}