#pragma once

#include "Engine/Graphics/GPUCommandBuffer.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUCommandBufferVulkan : public GPUResouceVulkan<Graphics::GPUCommandBuffer>
	{
	public:
		GPUCommandBufferVulkan();
		~GPUCommandBufferVulkan();

		virtual void Init(Graphics::GPUCommandBufferDesc const& desc) override;
		virtual void BeginRecord() override;
		virtual void EndRecord() override;
		virtual void Submit() override;
		virtual void Clear() override;

		virtual void OnReleaseGPU() override;

	private:
		VkCommandBuffer m_cmdBuffer;
	};
}