#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/RenderGraph/RenderGraphVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/GPUImageVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUSwapchainVulkan.h"
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

				if (renderPass.GetPassQueue() == Graphics::RenderPassQueue::Default)
				{
					attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				}
				else
				{
					attach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
					attach.initialLayout = ToVulkanImageLayout(res.TextureInfo.ImageLayout);
				}

				attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				attach.format = ToVulkanFormat(res.TextureInfo.m_info.Format);
				attach.finalLayout = ToVulkanImageLayout(res.TextureInfo.ImageLayout);
				attachmentDesc.push_back(attach);


				if (res.TextureInfo.ImageLayout == ImageLayout::Color_Attachment || res.TextureInfo.ImageLayout == ImageLayout::Shader_Read_Only)
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
				if (renderPass.GetPassQueue() == Graphics::RenderPassQueue::Default)
				{
					attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				}
				else
				{
					attach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
					attach.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}

				attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				attach.format = ToVulkanFormat(res.TextureInfo.m_info.Format);
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
			std::array<VkSubpassDependency, 1> dependencies;

			dependencies[0] = {};
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
			dependencies[0].srcAccessMask = 0;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			//dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			//dependencies[1].srcSubpass = 0;
			//dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			//dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			//dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			//dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			//dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			//dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.pAttachments = attachmentDesc.data();
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDesc.size());
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = (u32)dependencies.size();
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

	void GPURenderGraphPassVulkan::InitForSwapchain(Insight::Graphics::GPUSwapchain* swapchain, Insight::Graphics::GPUImage* image)
	{
		IS_PROFILE_FUNCTION();

		m_colorAttachmentsCount = 1;
		m_swapchainPass = true;
		GPUSwapchainVulkan* swapchainVulkan = static_cast<GPUSwapchainVulkan*>(swapchain);

		std::array<VkAttachmentDescription, 1> attachments = {};
		// Color attachment
		attachments[0].format = swapchainVulkan->GetColorFormat();
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		// Subpass dependencies for layout transitions
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
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VkDevice device = static_cast<GPUDeviceVulkan*>(GPUDevice::Instance())->Device;
		ThrowIfFailed(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass));

		VkImageView views[] = { static_cast<GPUImageViewVulkan*>(image->GetView())->GetImageView() };
		VkFramebufferCreateInfo fbufCreateInfo = {};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.pNext = NULL;
		fbufCreateInfo.renderPass = m_renderPass;
		fbufCreateInfo.pAttachments = views;
		fbufCreateInfo.attachmentCount = static_cast<u32>(1);
		fbufCreateInfo.width = static_cast<u32>(image->GetDesc().Width);
		fbufCreateInfo.height = static_cast<u32>(image->GetDesc().Height);
		fbufCreateInfo.layers = 1;
		ThrowIfFailed(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &m_frameBuffer));
	}
}

