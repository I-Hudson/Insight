#pragma once

#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"

namespace Insight::Graphics
{
	class RenderGraph;
	class GPUImage;
}

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUCommandBufferVulkan;

	class GPURenderGraphPassVulkan : public Graphics::GPURenderGraphPass
	{
	public:
		virtual ~GPURenderGraphPassVulkan();
		virtual void Init(Graphics::RenderPass& renderPass) override;

		VkRenderPass GetRenderPassVulkan() { return m_renderPass; }
		VkFramebuffer GetFramebufferVulkan() { return m_frameBuffer; }

	private:
		virtual void InitForSwapchain(Insight::Graphics::GPUSwapchain* swapchain, Insight::Graphics::GPUImage* image) override;

	private:
		VkRenderPass m_renderPass;
		VkFramebuffer m_frameBuffer;

		friend GPUCommandBufferVulkan;
	};

	class RenderGraphVulkan : public Graphics::RenderGraph
	{
	public:
	protected:
	};
}

