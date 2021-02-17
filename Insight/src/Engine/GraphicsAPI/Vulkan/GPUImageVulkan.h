#pragma once

#include "GPUDeviceVulkan.h"
#include "Engine/Graphics/Image/GPUImage.h"

class GPUImageViewVulkan : public GPUResouceVulkan<GPUImageView>
{
public:
	GPUImageViewVulkan();
	virtual ~GPUImageViewVulkan();

protected:
	//[GPUImageView]
	virtual bool OnInit() override;

protected:
	//[GPUResouce]
	virtual void OnReleaseGPU() override;

private:
	VkImageView m_vView;
};

class GPUImageVulkan : public GPUResouceVulkan<GPUImage>
{
public:
	GPUImageVulkan();
	virtual ~GPUImageVulkan();

	VkImage GetVulkanImage() const { return m_vImage; }

protected:
	//[GPUImage]
	virtual bool OnInit() override;

protected:
	//[GPUResouce]
	virtual void OnReleaseGPU() override;

private:
	VkImage m_vImage;
	VmaAllocation m_vImageAlloc;
};

