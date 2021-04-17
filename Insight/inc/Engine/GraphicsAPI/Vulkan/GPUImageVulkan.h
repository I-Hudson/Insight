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

		friend GPUCommandBufferVulkan;
	};
}