#pragma once

#include "GPUDeviceVulkan.h"
#include "Engine/Graphics/Image/GPUImage.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUCommandBufferVulkan;

	class GPUImageViewVulkan : public GPUResouceVulkan<Graphics::GPUImageView>
	{
	public:
		GPUImageViewVulkan();
		virtual ~GPUImageViewVulkan();

		VkImageView GetImageView() const { return m_vView; }

		//[GPUResouce]
		virtual void SetName(const std::string& name) override;

	protected:
		//[GPUImageView]
		virtual bool OnInit() override;

	protected:
		//[GPUResouce]
		virtual void OnReleaseGPU() override;

	private:
		VkImageView m_vView;
	};

	class GPUImageVulkan : public GPUResouceVulkan<Graphics::GPUImage>
	{
	public:
		GPUImageVulkan();
		virtual ~GPUImageVulkan();

		VkImage GetVulkanImage() const { return m_vImage; }
		VkDescriptorImageInfo* GetDescriptorImageInfo() { return &m_descriptorImageInfo; }

		//[GPUResouce]
		virtual void SetName(const std::string& name) override;

	protected:
		//[GPUImage]
		virtual bool OnInit() override;

	protected:
		//[GPUResouce]
		virtual void OnReleaseGPU() override;

	private:
		VkImage m_vImage;
		VmaAllocation m_vmaImageAlloc;
		VmaAllocationInfo m_vmaImageAllocInfo;
		VkDescriptorImageInfo m_descriptorImageInfo;

		friend GPUCommandBufferVulkan;
	};

	class GPUSamplerVulkan : public GPUResouceVulkan<Graphics::GPUSampler>
	{
	public:
		GPUSamplerVulkan();
		virtual ~GPUSamplerVulkan() override;

		virtual void Init(Graphics::GPUSamplerDesc& desc) override;

		VkSampler GetSampler() const { return m_sampler; }

		//[GPUResouce]
		virtual void SetName(const std::string& name) override;
	protected:
		virtual void OnReleaseGPU() override;

	private:
		VkSampler m_sampler;
	};
}