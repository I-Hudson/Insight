#pragma once

#include "Engine/Graphics/GPUDynamicBuffer.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUBufferVulkan;
	class GPUDynamicBufferVulkan : public GPUResouceVulkan<Graphics::GPUDynamicBuffer>
	{
	public:
		GPUDynamicBufferVulkan();
		virtual ~GPUDynamicBufferVulkan() override;

		virtual void Init(Graphics::GPUDynamicBufferDesc& desc) override;
		virtual Graphics::GPUBuffer* Upload(void* data, u64 size) override;

		virtual void SetName(const std::string& name) override;

	protected:
		//[GPUResource]
		virtual void OnReleaseGPU() override;

	private:
		u64 PadData(u64 orginalSize);

	private:
		VkBuffer m_buffer;
		VmaAllocation m_vmaAllocation;
		VmaAllocationInfo m_vmaAllocationInfo;

		friend GPUBufferVulkan;
	};
}