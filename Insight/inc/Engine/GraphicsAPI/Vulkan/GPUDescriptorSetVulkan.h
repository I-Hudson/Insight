#pragma once
#include "Engine/Graphics/GPUDescriptorSet.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUDescriptorPoolVulkan;
	class GPUDescriptorAllocatorVulkan;
	class GPUDescriptorLayoutCacheVulkan;
	class GPUDescriptorBuilderVulkan;

	class GPUDescriptorSetVulkan : public GPUResouceVulkan<Graphics::GPUDescriptorSet>
	{
	public:
		GPUDescriptorSetVulkan();
		virtual ~GPUDescriptorSetVulkan() override;

		virtual GPUResults Init(Graphics::GPUDescriptorSetDesc& desc) override;

		virtual void BindTexture(Graphics::GPUImage* image, u32 slot) override;
		virtual void BindUniformBuffer(Graphics::GPUBuffer* buffer, u32 slot) override;
		virtual void BindDynamicUniformBuffer(Graphics::GPUBuffer* buffer, u32 slot) override;

	protected:
		// [GPUResource]
		virtual void SetName(const std::string& name) override;
		virtual void OnReleaseGPU() override;

	private:
		VkDescriptorSet m_set;
		friend class GPUDescriptorPoolVulkan;
		friend class GPUDescriptorBuilderVulkan;
		friend class GPUDescriptorLayoutCacheVulkan;
		friend class GPUDescriptorAllocatorVulkan;
	};

	class GPUDescriptorPoolVulkan : public GPUResouceVulkan<Graphics::GPUDescriptorPool>
	{
	public:
		GPUDescriptorPoolVulkan();
		virtual ~GPUDescriptorPoolVulkan() override;

		virtual GPUResults Init() override;

		virtual Graphics::GPUDescriptorSet* AllocaSet(Graphics::GPUDescriptorSetDesc& desc) override;
		virtual void ResetPool(Graphics::GPUDescriptorPoolResetFlag resetFlags) override;

	protected:
		// [GPUResource]
		virtual void SetName(const std::string& name) override;
		virtual void OnReleaseGPU() override;

	private:
		VkDescriptorPool m_pool;

		friend class GPUDescriptorSetVulkan;
	};
}