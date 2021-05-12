#pragma once
#include "Engine/Core/Core.h"
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
		u32 ImageUsage;
	};

	struct RenderPassInfo
	{
		u32 ColorAttachmentCount = 0;
		u32 SamplerCount = 1;
	};

	class VulkanFrameBuffer
	{
	public:
		VulkanFrameBuffer();
		~VulkanFrameBuffer();

		void SetRect(const u32& width, const u32& height);
		void CreateAttachment(const u32& format, const u32& imageUsage, const std::string& attachmentName);
		void CreateRenderPass();

		void SetSampleCount(const u32& sampleCount, const bool& rebuild = true);

		const FrameBufferAttachment& GetAttachment(const std::string& attachmentName);
		std::vector<VkClearValue> GetClearAttachments();
		void WaitForFence();
		void ResetFence();

		const u32& GetWidth() const { return m_width; }
		const u32& GetHeight() const { return m_height; }
		VkFramebuffer GetFrameBuffer() const { return m_frameBuffer; }
		VkRenderPass GetRenderPass() const { return m_renderPass; }
		const RenderPassInfo& GetRenderPassInfo() const { return m_renderPassInfo; }
		VkSampler GetSampler() const { return m_sampler; }

		VkSemaphore GetReadySemaphore() const { return m_readySemaphore; }
		VkSemaphore GetFinishedSemaphore() const { return m_finishedSemaphore; }
		VkFence GetFence() const { return m_fence; }

	private:
		void CreateAttachment(FrameBufferAttachment& attachment);

	private:
		u32 m_width;
		u32 m_height;

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