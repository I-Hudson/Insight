#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/VulkanFramebuffer.h"
#include "Insight/Renderer/Vulkan/Device.h"

#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Event/EventManager.h"

namespace Insight
{
	namespace Render
	{
		VulkanFramebuffer::VulkanFramebuffer(Device* device, const int& width, const int& height)
			: m_device(device), m_extent(VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) })
		{
			m_imageAvailableSem = NEW_ON_HEAP(Semaphore, m_device);
			m_imageFinishedSem = NEW_ON_HEAP(Semaphore, m_device);

			m_fence = NEW_ON_HEAP(Fence, m_device);
		}

		VulkanFramebuffer::VulkanFramebuffer(Device* device, VkExtent2D extent)
			: m_device(device), m_extent(extent)
		{
			m_imageAvailableSem = NEW_ON_HEAP(Semaphore, m_device);
			m_imageFinishedSem = NEW_ON_HEAP(Semaphore, m_device);

			m_fence = NEW_ON_HEAP(Fence, m_device);
		}

		VulkanFramebuffer::VulkanFramebuffer(Device* device, VkImage* image, VkFormat format, VkExtent2D extent)
			: m_device(device), m_extent(extent)
		{
			m_imageAvailableSem = NEW_ON_HEAP(Semaphore, m_device);
			m_imageFinishedSem = NEW_ON_HEAP(Semaphore, m_device);

			m_fence = NEW_ON_HEAP(Fence, m_device);
		}

		VulkanFramebuffer::~VulkanFramebuffer()
		{
			DELETE_ON_HEAP(m_fence);

			DELETE_ON_HEAP(m_imageAvailableSem);
			DELETE_ON_HEAP(m_imageFinishedSem);

			DELETE_ON_HEAP(m_renderpass);

			vkDestroyFramebuffer(m_device->GetDevice(), m_frameBuffer, nullptr);

			for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it)
			{
				vkDestroyImageView(m_device->GetDevice(), (*it).View, nullptr);
				if ((*it).DeleteImage)
				{
					vkDestroyImage(m_device->GetDevice(), (*it).Image, nullptr);
					vkFreeMemory(m_device->GetDevice(), (*it).Mem, nullptr);
				}
			}
			vkDestroySampler(m_device->GetDevice(), m_sampler, nullptr);
			UNTRACK_OBJECT(m_sampler);
		}

		void VulkanFramebuffer::CreateAttachment(VkFormat format, VkImageUsageFlags usage, const VkImageLayout& imageLayout, const VkImageLayout& finalLayout)
		{
			VkImageAspectFlags aspectMask = 0;

			FrameBufferAttachment attachment;
			attachment.Format = format;
			attachment.ImageLayout = imageLayout;
			attachment.FinalLayout = finalLayout;
			attachment.DeleteImage = true;
			attachment.ImageUsage = usage;

			if (attachment.ImageUsage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			{
				attachment.ImageViewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			if (attachment.ImageUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				attachment.ImageViewAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			}

			m_attachments.push_back(attachment);
		}

		void VulkanFramebuffer::CompileFrameBuffer()
		{
			std::vector<VkImageView> views;
			for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it)
			{
				CreateImage(*it);
				CreateMemory(*it);
				CreateImageView(*it);

				views.push_back((*it).View);
			}

			CreateSampler();
			m_renderpass = NEW_ON_HEAP(Renderpass, m_device, m_attachments);

			VkFramebufferCreateInfo framebufferCreateInfo = VulkanInits::FramebufferInfo(m_renderpass->GetRenderpass(), views, m_extent.width, m_extent.height);
			ThrowIfFailed(vkCreateFramebuffer(m_device->GetDevice(), &framebufferCreateInfo, nullptr, &m_frameBuffer));
		}

		FrameBufferAttachment& VulkanFramebuffer::GetAttachment(const int& index)
		{
			return m_attachments[index];
		}

		void VulkanFramebuffer::AttachImage(VkImage* image, VkFormat format, const VkImageLayout& imageLayout, const VkImageLayout& finalLayout)
		{
			FrameBufferAttachment attachment;
			attachment.Image = *image;
			attachment.Format = format;
			attachment.ImageLayout = imageLayout;
			attachment.FinalLayout = finalLayout;
			attachment.DeleteImage = false;
			attachment.ImageViewAspect = VK_IMAGE_ASPECT_COLOR_BIT;

			m_attachments.push_back(attachment);
		}

		void VulkanFramebuffer::BindBuffer(CommandBuffer* commandBuffers, const VkSubpassContents& subpassContents)
		{
			IS_PROFILE_FUNCTION();
			std::vector<VkClearValue> clearColours;
			for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it)
			{
				if ((*it).Format == VK_FORMAT_D32_SFLOAT)
				{
					VkClearValue clearValue;
					clearValue.depthStencil = { static_cast<uint32_t>(1.0f), static_cast<uint32_t>(0.0f) };
					clearColours.push_back(clearValue);
				}
				else
				{
					VkClearValue clearValue;
					clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };
					clearColours.push_back(clearValue);
				}
			}
			VkRenderPassBeginInfo renderpassInfo = VulkanInits::RenderPassBeginInfo(m_renderpass->GetRenderpass(), m_frameBuffer, m_extent, clearColours);

			vkCmdBeginRenderPass(commandBuffers->GetBuffer(), &renderpassInfo, subpassContents);
		}

		void VulkanFramebuffer::UnbindBuffer(CommandBuffer* commandBuffers)
		{
			IS_PROFILE_FUNCTION();
			vkCmdEndRenderPass(commandBuffers->GetBuffer());
		}

		void VulkanFramebuffer::SetRenderPass(Renderpass* renderpass)
		{
			m_renderpass = renderpass;
		}

		void VulkanFramebuffer::Resize(int width, int height)
		{
			vkDestroyFramebuffer(m_device->GetDevice(), m_frameBuffer, nullptr);

			for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it)
			{
				vkDestroyImageView(m_device->GetDevice(), (*it).View, nullptr);
				if ((*it).DeleteImage)
				{
					vkDestroyImage(m_device->GetDevice(), (*it).Image, nullptr);
					vkFreeMemory(m_device->GetDevice(), (*it).Mem, nullptr);
				}
			}

			vkDestroySampler(m_device->GetDevice(), m_sampler, nullptr);
			UNTRACK_OBJECT(m_sampler);

			m_extent = VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			std::vector<VkImageView> views;
			for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it)
			{
				CreateImage(*it);
				CreateMemory(*it);
				CreateImageView(*it);

				views.push_back((*it).View);
			}

			CreateSampler();
			m_renderpass->Recreate(m_attachments);

			VkFramebufferCreateInfo framebufferCreateInfo = VulkanInits::FramebufferInfo(m_renderpass->GetRenderpass(), views, m_extent.width, m_extent.height);
			ThrowIfFailed(vkCreateFramebuffer(m_device->GetDevice(), &framebufferCreateInfo, nullptr, &m_frameBuffer));
		}

		void VulkanFramebuffer::CreateImage(FrameBufferAttachment& attachment)
		{
			if (attachment.DeleteImage)
			{
				VkImageCreateInfo imageCreateInfo = VulkanInits::ImageCreateInfo(attachment.Format, m_extent, attachment.ImageUsage);
				ThrowIfFailed(vkCreateImage(m_device->GetDevice(), &imageCreateInfo, nullptr, &attachment.Image));
			}
		}

		void VulkanFramebuffer::CreateImageView(FrameBufferAttachment& attachment)
		{
			VkImageViewCreateInfo createInfo = VulkanInits::ImageViewInfo();
			createInfo.image = attachment.Image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = attachment.Format;
			createInfo.subresourceRange.aspectMask = attachment.ImageViewAspect;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			ThrowIfFailed(vkCreateImageView(m_device->GetDevice(), &createInfo, nullptr, &attachment.View));
		}

		void VulkanFramebuffer::CreateMemory(FrameBufferAttachment& attachment)
		{
			if (attachment.DeleteImage)
			{
				VkMemoryAllocateInfo allocInfo = VulkanInits::MemoryAllocInfo();
				VkMemoryRequirements memReq;
				vkGetImageMemoryRequirements(m_device->GetDevice(), attachment.Image, &memReq);
				allocInfo.allocationSize = memReq.size;
				allocInfo.memoryTypeIndex = m_device->GetMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				ThrowIfFailed(vkAllocateMemory(m_device->GetDevice(), &allocInfo, nullptr, &attachment.Mem));
				ThrowIfFailed(vkBindImageMemory(m_device->GetDevice(), attachment.Image, attachment.Mem, 0));
			}
		}
		void VulkanFramebuffer::CreateSampler()
		{
			// Create sampler to sample from the color attachments
			VkSamplerCreateInfo sampler = VulkanInits::Sampler();
			sampler.magFilter = VK_FILTER_NEAREST;
			sampler.minFilter = VK_FILTER_NEAREST;
			sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeV = sampler.addressModeU;
			sampler.addressModeW = sampler.addressModeU;
			sampler.mipLodBias = 0.0f;
			sampler.maxAnisotropy = 1.0f;
			sampler.minLod = 0.0f;
			sampler.maxLod = 1.0f;
			sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			ThrowIfFailed(vkCreateSampler(m_device->GetDevice(), &sampler, nullptr, &m_sampler));
			TRACK_OBJECT(m_sampler);
		}
	}
}
#endif // IS_VULKAN