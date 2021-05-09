#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/RenderGraph/RenderGraphVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/GPUImageVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"

namespace Insight::GraphicsAPI::Vulkan
{
	GPURenderGraphPassVulkan::~GPURenderGraphPassVulkan()
	{
		VkDevice device = static_cast<GPUDeviceVulkan*>(GPUDevice::Instance())->Device;

		vkDestroyFramebuffer(device, m_frameBuffer, nullptr);
		vkDestroyRenderPass(device, m_renderPass, nullptr);
	}

	void GPURenderGraphPassVulkan::Init(Graphics::RenderPass& renderPass)
	{
		VkDevice device = static_cast<GPUDeviceVulkan*>(GPUDevice::Instance())->Device;

		renderPass.SetGPUGraphPass(this);
		m_renderPassIndex = renderPass.GetPassIndex();
		m_graph = renderPass.GetGraph();
		// Setup all our attachments for the render pass.
		std::vector<VkAttachmentDescription> attachmentDesc = { };
		std::vector<VkAttachmentReference> colourAttachRefs;
		VkAttachmentReference depthAttachRefs = {};
		std::vector<VkImageView> fbAttachments = {};
		VkSubpassDescription subpass = {};

		{
			IS_PROFILE_SCOPE("Create Vulkan Render Pass");

			u32 index = 0;
			m_colorAttachmentsCount = static_cast<u32>(renderPass.GetColorOutputs().size());
			for (auto& attIndex : renderPass.GetColorOutputs())
			{
				auto& res = renderPass.GetTextureResource(attIndex);
				VkAttachmentDescription attach = {};
				attach.samples = ToVulkanSampleCount(res.TextureInfo.m_info.Samples);
				attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				attach.format = ToVulkanFormat(res.TextureInfo.m_info.Format);
				attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attach.finalLayout = ToVulkanImageLayout(res.TextureInfo.ImageLayout);
				attachmentDesc.push_back(attach);

				if (res.TextureInfo.ImageLayout == ImageLayout::Color_Attachment)
				{
					colourAttachRefs.push_back({ index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
				}
				const GPUImageViewVulkan* imageView = static_cast<const GPUImageViewVulkan*>(renderPass.GetPhysicalImageView(res.GetPhysicalIndex()));
				fbAttachments.push_back(imageView->GetImageView());
				++index;
			}

			if (renderPass.IsDepthSencilOuputValid())
			{
				depthAttachRefs.attachment = static_cast<u32>(fbAttachments.size());
				depthAttachRefs.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


				auto& res = renderPass.GetTextureResource(renderPass.GetDepthStencilOutput().GetIndex());
				VkAttachmentDescription attach = {};
				attach.samples = ToVulkanSampleCount(res.TextureInfo.m_info.Samples);
				attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				attach.format = ToVulkanFormat(res.TextureInfo.m_info.Format);
				attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attach.finalLayout = ToVulkanImageLayout(res.TextureInfo.ImageLayout);
				attachmentDesc.push_back(attach);

				const GPUImageViewVulkan* imageView = static_cast<const GPUImageViewVulkan*>(renderPass.GetPhysicalImageView(res.GetPhysicalIndex()));
				fbAttachments.push_back(imageView->GetImageView());

				subpass.pDepthStencilAttachment = &depthAttachRefs;
			}

			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.pColorAttachments = colourAttachRefs.data();
			subpass.colorAttachmentCount = static_cast<uint32_t>(colourAttachRefs.size());

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


			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.pAttachments = attachmentDesc.data();
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDesc.size());
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 2;
			renderPassInfo.pDependencies = dependencies.data();

			ThrowIfFailed(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass));
		}

		VkFramebufferCreateInfo fbufCreateInfo = {};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.pNext = NULL;
		fbufCreateInfo.renderPass = m_renderPass;
		fbufCreateInfo.pAttachments = fbAttachments.data();
		fbufCreateInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
		fbufCreateInfo.width = static_cast<u32>(renderPass.GetWindowRect().GetWidth());
		fbufCreateInfo.height = static_cast<u32>(renderPass.GetWindowRect().GetHeight());
		fbufCreateInfo.layers = 1;
		ThrowIfFailed(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &m_frameBuffer));
	}
}

