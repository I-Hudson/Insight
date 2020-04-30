#include "ispch.h"
#include "Framebuffer.h"

#include "Insight/Renderer/Lowlevel/Device.h"
#include "Insight/Renderer/VulkanInits.h"
#include "Insight/Memory/MemoryManager.h"

#include "Insight/Event/EventManager.h"

namespace Insight
{
	namespace Render
	{
		Framebuffer::Framebuffer(Device* device, const int& width, const int& height)
			: m_device(device), m_extent(VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) })
		{
			m_imageAvailableSem = Memory::MemoryManager::NewOnFreeList<Semaphore>(m_device);
			m_imageFinishedSem = Memory::MemoryManager::NewOnFreeList<Semaphore>(m_device);

			m_fence = Memory::MemoryManager::NewOnFreeList<Fence>(m_device);
		}

		Framebuffer::Framebuffer(Device* device, VkExtent2D extent)
			: m_device(device), m_extent(extent)
		{
			m_imageAvailableSem = Memory::MemoryManager::NewOnFreeList<Semaphore>(m_device);
			m_imageFinishedSem = Memory::MemoryManager::NewOnFreeList<Semaphore>(m_device);

			m_fence = Memory::MemoryManager::NewOnFreeList<Fence>(m_device);
		}

		Framebuffer::Framebuffer(Device* device, VkImage* image, VkFormat format, VkExtent2D extent)
			: m_device(device), m_extent(extent)
		{
			m_imageAvailableSem = Memory::MemoryManager::NewOnFreeList<Semaphore>(m_device);
			m_imageFinishedSem = Memory::MemoryManager::NewOnFreeList<Semaphore>(m_device);

			m_fence = Memory::MemoryManager::NewOnFreeList<Fence>(m_device);
		}

		Framebuffer::~Framebuffer()
		{
			Memory::MemoryManager::DeleteOnFreeList(m_fence);

			Memory::MemoryManager::DeleteOnFreeList(m_imageAvailableSem);
			Memory::MemoryManager::DeleteOnFreeList(m_imageFinishedSem);

			Memory::MemoryManager::DeleteOnFreeList(m_renderpass);

			vkDestroyFramebuffer(m_device->GetDevice(), m_frameBuffer, nullptr);

			for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it)
			{
				vkDestroyImageView(m_device->GetDevice(), (*it).View, nullptr);
				if ((*it).DeleteImage)
				{
					vkDestroyImage(m_device->GetDevice(), (*it).Image, nullptr);
				}
			}
		}

		void Framebuffer::CreateAttachment(VkFormat format, VkImageUsageFlags usage, const VkImageLayout& imageLayout, const VkImageLayout& finalLayout)
		{
			VkImageAspectFlags aspectMask = 0;

			FrameBufferAttachment attachment;
			attachment.Format = format;
			attachment.ImageLayout = imageLayout;
			attachment.FinalLayout = finalLayout;
			attachment.DeleteImage = true;

			if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			{
				aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			VkImageCreateInfo imageCreateInfo = VulkanInits::ImageCreateInfo(attachment.Format, m_extent, usage);
			ThrowIfFailed(vkCreateImage(m_device->GetDevice(), &imageCreateInfo, nullptr, &attachment.Image));

			VkMemoryAllocateInfo allocInfo = VulkanInits::MemoryAllocInfo();
			VkMemoryRequirements memReq;
			vkGetImageMemoryRequirements(m_device->GetDevice(), attachment.Image, &memReq);
			allocInfo.allocationSize = memReq.size;
			allocInfo.memoryTypeIndex = m_device->GetMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			ThrowIfFailed(vkAllocateMemory(m_device->GetDevice(), &allocInfo, nullptr, &attachment.Mem));
			ThrowIfFailed(vkBindImageMemory(m_device->GetDevice(), attachment.Image, attachment.Mem, 0));

			VkImageViewCreateInfo createInfo = VulkanInits::ImageViewInfo();
			createInfo.image = attachment.Image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = attachment.Format;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			ThrowIfFailed(vkCreateImageView(m_device->GetDevice(), &createInfo, nullptr, &attachment.View));

			m_attachments.push_back(attachment);
		}

		void Framebuffer::CompileFrameBuffer()
		{
			m_renderpass = Memory::MemoryManager::NewOnFreeList<Renderpass>(m_device, m_attachments);

			std::vector<VkImageView> views;
			for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it)
			{
				views.push_back((*it).View);
			}

			VkFramebufferCreateInfo framebufferCreateInfo = VulkanInits::FramebufferInfo(m_renderpass->GetRenderpass(), views, m_extent.width, m_extent.height);
			ThrowIfFailed(vkCreateFramebuffer(m_device->GetDevice(), &framebufferCreateInfo, nullptr, &m_frameBuffer));
		}

		void Framebuffer::AttachImage(VkImage* image, VkFormat format, const VkImageLayout& imageLayout, const VkImageLayout& finalLayout)
		{
			FrameBufferAttachment attachment;
			attachment.Image = *image;
			attachment.Format = format;
			attachment.ImageLayout = imageLayout;
			attachment.FinalLayout = finalLayout;
			attachment.DeleteImage = false;

			VkImageViewCreateInfo createInfo = VulkanInits::ImageViewInfo();
			createInfo.image = attachment.Image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = attachment.Format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			ThrowIfFailed(vkCreateImageView(m_device->GetDevice(), &createInfo, nullptr, &attachment.View));

			m_attachments.push_back(attachment);
		}

		void Framebuffer::BindBuffer(CommandBuffer* commandBuffers, const VkSubpassContents& subpassContents)
		{
			std::vector<VkClearValue> clearColours;
			for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it)
			{
				clearColours.push_back(VkClearValue{ 0.0f, 0.0f, 0.0f, 0.0f });
			}
			VkRenderPassBeginInfo renderpassInfo = VulkanInits::RenderPassBeginInfo(m_renderpass->GetRenderpass(), m_frameBuffer, m_extent, clearColours);

			vkCmdBeginRenderPass(commandBuffers->GetBuffer(), &renderpassInfo, subpassContents);
		}

		void Framebuffer::UnbindBuffer(CommandBuffer* commandBuffers)
		{
			vkCmdEndRenderPass(commandBuffers->GetBuffer());
		}

		void Framebuffer::SetRenderPass(Renderpass* renderpass)
		{
			m_renderpass = renderpass;
		}

		void Framebuffer::Resize(int width, int height)
		{
			m_renderpass->Recreate(m_attachments);
		}
	}
}