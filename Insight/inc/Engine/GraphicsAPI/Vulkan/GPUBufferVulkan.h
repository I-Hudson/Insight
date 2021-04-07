#pragma once

#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUBufferVulkan : public GPUResouceVulkan<Graphics::GPUBuffer>
	{
	public:
		GPUBufferVulkan();
		~GPUBufferVulkan();

		virtual void Init(Graphics::GPUBufferDesc const& desc) override;

	protected:
		virtual void* Map(GPUResourceMapMode mapMode) override;
		virtual void UnMap() override;

		virtual void Upload() override;
		virtual void Download() override;

		// [GPUResource]
		virtual void OnReleaseGPU() override;

	private:
		VkBuffer m_buffer;
		VmaAllocation m_vmaAllocation;
		VmaAllocationInfo m_vmaAllocationInfo;
	};
}