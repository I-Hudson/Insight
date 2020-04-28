#include "ispch.h"
#include "Renderpass.h"

#include "Insight/Renderer/Lowlevel/Device.h"

namespace Insight
{
	namespace Render
	{
		Renderpass::Renderpass(const Device* device, const RenderpassData& data)
			: m_device(device)
		{
			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


			auto attachments = GetAttachments(data.Attachments);
			auto attachmentRefs = GetAttachmentReferences(data.Attachments);
			auto subpass = GetSubpass(data.PiplineBindPoint, attachmentRefs);
			std::vector<VkSubpassDependency> dependecies = { dependency };
			VkRenderPassCreateInfo renderPassCreateInfo = VulkanInits::RenderPassInfo(attachments, subpass, dependecies);

			ThrowIfFailed(vkCreateRenderPass(m_device->GetDevice(), &renderPassCreateInfo, nullptr, &m_renderPass));
		}

		Renderpass::~Renderpass()
		{
			vkDestroyRenderPass(m_device->GetDevice(), m_renderPass, nullptr);
		}

		std::vector<VkAttachmentDescription> Renderpass::GetAttachments(const std::vector<ImageAttachment>& attachments)
		{
			std::vector<VkAttachmentDescription> attachs{};

			for (auto it = attachments.begin(); it != attachments.end(); ++it)
			{
				VkAttachmentDescription attachment{};
				attachment.format = (*it).AttchFormat;
				attachment.samples = (*it).AttchSamples;
				attachment.loadOp = (*it).AttchLoadOp;
				attachment.storeOp = (*it).AttchStoreOp;
				attachment.stencilLoadOp = (*it).AttchStencilLoadOp;
				attachment.stencilStoreOp = (*it).AttchStencilStoreOp;
				attachment.initialLayout = (*it).AttchInitalLayout;
				attachment.finalLayout = (*it).AttchFinalLayout;

				attachs.push_back(attachment);
			}

			return attachs;
		}

		std::vector<VkAttachmentReference> Renderpass::GetAttachmentReferences(const std::vector<ImageAttachment>& attachments)
		{
			std::vector<VkAttachmentReference> attachs{};

			int index = 0;
			for (auto it = attachments.begin(); it != attachments.end(); ++it)
			{
				VkAttachmentReference attachmentRef{};
				attachmentRef.attachment = index++;
				attachmentRef.layout = (*it).RefLayout;

				attachs.push_back(attachmentRef);
			}

			return attachs;
		}

		VkSubpassDescription Renderpass::GetSubpass(const VkPipelineBindPoint& bindPoint, const std::vector<VkAttachmentReference>& attachmentRefs)
		{
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = bindPoint;
			subpass.colorAttachmentCount = static_cast<uint32_t>(attachmentRefs.size());
			subpass.pColorAttachments = attachmentRefs.data();
			return subpass;
		}
	}
}