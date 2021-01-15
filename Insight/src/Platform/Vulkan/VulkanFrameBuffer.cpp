#include "ispch.h"
#include "VulkanFrameBuffer.h"
#include "VulkanDevice.h"

namespace vks
{
	VulkanFrameBuffer::VulkanFrameBuffer()
		: m_destroyed(false)
	{ }

	VulkanFrameBuffer::~VulkanFrameBuffer()
	{
		if (!m_destroyed)
		{
			auto device = VulkanDevice::Instance();
			vkDestroySampler(*device, m_sampler, nullptr);

			for (auto& attachment : m_attachmnets)
			{
				vkDestroyImageView(*device, attachment.ImageView, nullptr);
				vkDestroyImage(*device, attachment.Image, nullptr);
				vkFreeMemory(*device, attachment.DeviceMemory, nullptr);
			}

			vkDestroyFramebuffer(*device, m_frameBuffer, nullptr);
			vkDestroyRenderPass(*device, m_renderPass, nullptr);
			vkDestroySemaphore(*device, m_semaphore, nullptr);
			m_destroyed = true;
		}
	}

	void VulkanFrameBuffer::SetRect(const U32& width, const U32& height)
	{
		m_width = width;
		m_height = height;
	}

	void VulkanFrameBuffer::CreateAttachment(const U32& format, const U32& imageUsage, const std::string& attachmentName)
	{
		VkImageAspectFlags aspectMask = 0;
		FrameBufferAttachment attachment;

		attachment.Name = attachmentName;
		attachment.Format = (VkFormat)format;

		if (imageUsage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			attachment.ImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		if (imageUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			attachment.ImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		IS_CORE_ASSERT((aspectMask > 0), "");

		VkImageCreateInfo image = vks::initializers::imageCreateInfo();
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = (VkFormat)format;
		image.extent.width = m_width;
		image.extent.height = m_height;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = imageUsage | VK_IMAGE_USAGE_SAMPLED_BIT;

		VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
		VkMemoryRequirements memReqs;

		auto device = VulkanDevice::Instance();
		ThrowIfFailed(vkCreateImage(*device, &image, nullptr, &attachment.Image));
		vkGetImageMemoryRequirements(*device, attachment.Image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = device->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		ThrowIfFailed(vkAllocateMemory(*device, &memAlloc, nullptr, &attachment.DeviceMemory));
		ThrowIfFailed(vkBindImageMemory(*device, attachment.Image, attachment.DeviceMemory, 0));

		VkImageViewCreateInfo imageView = vks::initializers::imageViewCreateInfo();
		imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageView.format = (VkFormat)format;
		imageView.subresourceRange = {};
		imageView.subresourceRange.aspectMask = aspectMask;
		imageView.subresourceRange.baseMipLevel = 0;
		imageView.subresourceRange.levelCount = 1;
		imageView.subresourceRange.baseArrayLayer = 0;
		imageView.subresourceRange.layerCount = 1;
		imageView.image = attachment.Image;
		ThrowIfFailed(vkCreateImageView(*device, &imageView, nullptr, &attachment.ImageView));
	
		m_attachmnets.push_back(attachment);
	}

	void VulkanFrameBuffer::CreateRenderPass()
	{
		const U32 attachmentsSize = m_attachmnets.size();
		// Set up separate renderpass with references to the color and depth attachments
		std::vector<VkAttachmentDescription> attachmentDescs;
		std::vector<VkImageView> attachmentsViews;
		attachmentDescs.resize(attachmentsSize);
		attachmentsViews.resize(attachmentsSize);

		std::vector<VkAttachmentReference> colorReferences;
		VkAttachmentReference depthReference = {};

		// Init attachment properties
		for (uint32_t i = 0; i < attachmentsSize; ++i)
		{
			attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			if (m_attachmnets[i].ImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthReference.attachment = i;
				depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			else
			{
				attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				colorReferences.push_back({ i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			}

			attachmentDescs[i].format = m_attachmnets[i].Format;
			attachmentsViews[i] = m_attachmnets[i].ImageView;
		}

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
		subpass.pDepthStencilAttachment = &depthReference;

		// Use subpass dependencies for attachment layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		auto device = VulkanDevice::Instance();

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachmentDescs.data();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();

		ThrowIfFailed(vkCreateRenderPass(*device, &renderPassInfo, nullptr, &m_renderPass));
	
		VkFramebufferCreateInfo fbufCreateInfo = {};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.pNext = NULL;
		fbufCreateInfo.renderPass = m_renderPass;
		fbufCreateInfo.pAttachments = attachmentsViews.data();
		fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentsViews.size());
		fbufCreateInfo.width = m_width;
		fbufCreateInfo.height = m_height;
		fbufCreateInfo.layers = 1;
		ThrowIfFailed(vkCreateFramebuffer(*device, &fbufCreateInfo, nullptr, &m_frameBuffer));
	
		// Create sampler to sample from the color attachments
		VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
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
		ThrowIfFailed(vkCreateSampler(*device, &sampler, nullptr, &m_sampler));

		// Create a semaphore used to synchronize offscreen rendering and usage
		VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
		ThrowIfFailed(vkCreateSemaphore(*device, &semaphoreCreateInfo, nullptr, &m_semaphore));
	}
}