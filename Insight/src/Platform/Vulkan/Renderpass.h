#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Vulkan.h"

namespace Platform
{
	class Device;
	struct FrameBufferAttachment;

	class IS_API Renderpass
	{
	public:
		Renderpass() = delete;
		Renderpass(const Device* device, const std::vector<FrameBufferAttachment>& fbAttachments);
		~Renderpass();

		const VkRenderPass& GetRenderpass() const { return m_renderPass; }
		void Recreate(const std::vector<FrameBufferAttachment>& fbAttachments);

	private:
		void Create(const std::vector<FrameBufferAttachment>& fbAttachments);

		std::vector<VkAttachmentDescription> GetAttachments(const std::vector<FrameBufferAttachment>& attachments);
		std::vector<VkAttachmentReference> GetAttachmentReferences(const std::vector<FrameBufferAttachment>& attachments, const VkImageLayout& mask, const int& offset);

		VkSubpassDescription GetSubpass(const VkPipelineBindPoint& bindPoint, const std::vector<VkAttachmentReference>& colourAttachmentRefs,
			const std::vector<VkAttachmentReference>& depthAttachmentRefs);

	private:
		const Device* m_device;
		VkRenderPass m_renderPass;
	};
}
#endif