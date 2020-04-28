#include "ispch.h"
#include "Framebuffer.h"

#include "Insight/Renderer/Lowlevel/Device.h"
#include "Insight/Renderer/VulkanInits.h"
#include "Insight/Memory/MemoryManager.h"

namespace Insight
{
	namespace Render
	{
		Framebuffer::Framebuffer(const Device* device, VkImage* image, VkFormat format, VkExtent2D extent, Renderpass* renderpass)
			: m_device(device), m_image(*image), m_format(format), m_extent(extent), m_renderpass(renderpass)
		{
			VkImageViewCreateInfo createInfo = VulkanInits::ImageViewInfo();
			createInfo.image = m_image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			ThrowIfFailed(vkCreateImageView(m_device->GetDevice(), &createInfo, nullptr, &m_view));

			VkImageView imageViews[] =
			{
				m_view
			};
			VkFramebufferCreateInfo framebufferCreateInfo = VulkanInits::FramebufferInfo(m_renderpass->GetRenderpass(), 1, imageViews, extent.width,
				extent.height);
			ThrowIfFailed(vkCreateFramebuffer(m_device->GetDevice(), &framebufferCreateInfo, nullptr, &m_frameBuffer));

			m_imageAvailableSem = Memory::MemoryManager::NewOnFreeList<Semaphore>(m_device);
			m_imageFinishedSem = Memory::MemoryManager::NewOnFreeList<Semaphore>(m_device);

			m_fence = Memory::MemoryManager::NewOnFreeList<Fence>(m_device);
		}

		Framebuffer::~Framebuffer()
		{
			Memory::MemoryManager::DeleteOnFreeList(m_fence);

			Memory::MemoryManager::DeleteOnFreeList(m_imageAvailableSem);
			Memory::MemoryManager::DeleteOnFreeList(m_imageFinishedSem);

			vkDestroyFramebuffer(m_device->GetDevice(), m_frameBuffer, nullptr);
			vkDestroyImageView(m_device->GetDevice(), m_view, nullptr);
		}

		void Framebuffer::BindBuffer(CommandBuffer* commandBuffers, const VkSubpassContents& subpassContents)
		{
			VkRenderPassBeginInfo renderpassInfo = VulkanInits::RenderPassBeginInfo(m_renderpass->GetRenderpass(), m_frameBuffer, m_extent, { 0.0f, 0.0f, 0.0f, 0.0f });

			vkCmdBeginRenderPass(commandBuffers->GetBuffer(), &renderpassInfo, subpassContents);
		}

		void Framebuffer::SetRenderPass(Renderpass* renderpass)
		{
			m_renderpass = renderpass;
		}
	}
}