#pragma once
#include "Insight/Core/Core.h"
#include "Platform/Vulkan/VulkanHeader.h"
namespace vks
{
	struct FrameBufferAttachment
	{
		std::string Name;
		VkImage Image;
		VkImageView ImageView;
		VkImageLayout ImageLayout;
		VkDeviceMemory DeviceMemory;
		VkFormat Format;
	};

	class VulkanFrameBuffer
	{
	public:
		VulkanFrameBuffer();
		~VulkanFrameBuffer();

		void SetRect(const U32& width, const U32& height);
		void CreateAttachment(const U32& format, const U32& imageUsage, const std::string& attachmentName);
		void CreateRenderPass();

		VkFramebuffer GetFrameBuffer() const { return m_frameBuffer; }
		VkRenderPass GetRenderPass() const { return m_renderPass; }
		VkSemaphore GetSemaphore() const { return m_semaphore; }

	private:
		U32 m_width;
		U32 m_height;

		VkFramebuffer m_frameBuffer;
		std::vector<FrameBufferAttachment> m_attachmnets;
		VkRenderPass m_renderPass;
		VkSampler m_sampler;

		VkSemaphore m_semaphore;
		bool m_destroyed;
	};
}