#include "ispch.h"
#include "Renderpass.h"

#include "Insight/Renderer/Lowlevel/Device.h"
#include "Insight\Renderer\Lowlevel\Framebuffer.h"

namespace Insight
{
	namespace Render
	{
		Renderpass::Renderpass(const Device* device, const std::vector<FrameBufferAttachment>& fbAttachments)
			: m_device(device)
		{
			Create(fbAttachments);
		}

		Renderpass::~Renderpass()
		{
			vkDestroyRenderPass(m_device->GetDevice(), m_renderPass, nullptr);
		}

		void Renderpass::Recreate(const std::vector<FrameBufferAttachment>& fbAttachments)
		{
			vkDestroyRenderPass(m_device->GetDevice(), m_renderPass, nullptr);
			Create(fbAttachments);
		}

		void Renderpass::Create(const std::vector<FrameBufferAttachment>& fbAttachments)
		{
			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


			auto attachments = GetAttachments(fbAttachments);
			auto cAttachmentRefs = GetAttachmentReferences(fbAttachments, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			auto dAttachmentRefs = GetAttachmentReferences(fbAttachments, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			auto subpass = GetSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS, cAttachmentRefs, dAttachmentRefs);
			std::vector<VkSubpassDependency> dependecies = { dependency };
			VkRenderPassCreateInfo renderPassCreateInfo = VulkanInits::RenderPassInfo(attachments, subpass, dependecies);

			ThrowIfFailed(vkCreateRenderPass(m_device->GetDevice(), &renderPassCreateInfo, nullptr, &m_renderPass));
		}

		std::vector<VkAttachmentDescription> Renderpass::GetAttachments(const std::vector<FrameBufferAttachment>& attachments)
		{
			std::vector<VkAttachmentDescription> attachs{};

			for (auto it = attachments.begin(); it != attachments.end(); ++it)
			{
				VkAttachmentDescription attachment{};
				attachment.format = (*it).Format;
				attachment.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachment.finalLayout = (*it).FinalLayout;

				attachs.push_back(attachment);
			}

			return attachs;
		}

		std::vector<VkAttachmentReference> Renderpass::GetAttachmentReferences(const std::vector<FrameBufferAttachment>& attachments, const VkImageLayout& mask)
		{
			std::vector<VkAttachmentReference> attachs{};

			int index = 0;
			for (auto it = attachments.begin(); it != attachments.end(); ++it)
			{
				if ((*it).ImageLayout == mask)
				{
					VkAttachmentReference attachmentRef{};
					attachmentRef.attachment = index++;
					attachmentRef.layout = (*it).ImageLayout;

					attachs.push_back(attachmentRef);
				}
			}

			return attachs;
		}

		VkSubpassDescription Renderpass::GetSubpass(const VkPipelineBindPoint& bindPoint, const std::vector<VkAttachmentReference>& colourAttachmentRefs, 
			const std::vector<VkAttachmentReference>& depthAttachmentRefs)
		{
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = bindPoint;
			subpass.colorAttachmentCount = static_cast<uint32_t>(colourAttachmentRefs.size());
			subpass.pColorAttachments = colourAttachmentRefs.data();
			subpass.pDepthStencilAttachment = depthAttachmentRefs.data();
			return subpass;
		}
	}
}