#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/VulkanInits.h"

#include "Insight/Renderer/Lowlevel/Renderpass.h"
#include "Insight/Renderer/Lowlevel/CommandBuffer.h"

namespace Insight
{
	namespace Render
	{
		class Device;

		struct ImageAttachment
		{
			VkImageLayout RefLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkFormat AttchFormat;
			VkSampleCountFlagBits AttchSamples = VK_SAMPLE_COUNT_1_BIT;

			VkAttachmentLoadOp AttchLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			VkAttachmentStoreOp AttchStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			VkAttachmentLoadOp AttchStencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			VkAttachmentStoreOp AttchStencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			VkImageLayout AttchInitalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkImageLayout AttchFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		};

		struct RenderpassData
		{
			std::vector<ImageAttachment>& Attachments;
			VkPipelineBindPoint PiplineBindPoint;
		};

		class IS_API Renderpass
		{
		public: 
			Renderpass() = delete;
			Renderpass(const Device* device, const RenderpassData& data);
			~Renderpass();

			const VkRenderPass& GetRenderpass() const { return m_renderPass; }

		private:
			std::vector<VkAttachmentDescription> GetAttachments(const std::vector<ImageAttachment>& attachments);
			std::vector<VkAttachmentReference> GetAttachmentReferences(const std::vector<ImageAttachment>& attachments);

			VkSubpassDescription GetSubpass(const VkPipelineBindPoint& bindPoint, const std::vector<VkAttachmentReference>& attachmentRefs);

		private:
			const Device* m_device;
			VkRenderPass m_renderPass;
		};
	}
}