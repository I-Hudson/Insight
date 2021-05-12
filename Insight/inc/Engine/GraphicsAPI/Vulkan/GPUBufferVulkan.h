#pragma once

#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUCommandBufferVulkan;

	class GPUBufferVulkan : public GPUResouceVulkan<Graphics::GPUBuffer>
	{
	public:
		GPUBufferVulkan();
		~GPUBufferVulkan();

		virtual void Init(Graphics::GPUBufferDesc const& desc) override;

		VkDescriptorBufferInfo* GetBufferInfo() { return &m_bufferInfo; }

	protected:
		virtual void* Map() override;
		virtual void UnMap() override;

		virtual void Upload(void* mapped, void const* data, u64 const& size) override;
		virtual void Download(std::vector<u8>& data, void const* mapped) override;

		// [GPUResource]
		virtual void OnReleaseGPU() override;
		virtual void SetName(const std::string& name) override;

	private:
		VkBuffer m_buffer;
		VmaAllocation m_vmaAllocation;
		VmaMemoryUsage m_vmaMemoryUsage;
		VmaAllocationInfo m_vmaAllocationInfo;

		VkDescriptorBufferInfo m_bufferInfo;

		friend GPUCommandBufferVulkan;
	};
}