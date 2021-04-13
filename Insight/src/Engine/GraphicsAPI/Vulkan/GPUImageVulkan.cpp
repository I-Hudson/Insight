#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUImageVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/Debug/GPUDebugVulkan.h"
#include "VulkanInitializers.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/Graphics/PixelFormatExtensions.h"

namespace Insight::GraphicsAPI::Vulkan
{
	GPUImageViewVulkan::GPUImageViewVulkan()
		: m_vView(VK_NULL_HANDLE)
	{
	}

	GPUImageViewVulkan::~GPUImageViewVulkan()
	{
		OnReleaseGPU();
	}

	bool GPUImageViewVulkan::OnInit()
	{
		if (m_image == nullptr)
		{
			return false;
		}
		VkImageViewCreateInfo info = vks::initializers::imageViewCreateInfo();
		info.image = static_cast<GPUImageVulkan*>(m_image)->GetVulkanImage();
		info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.subresourceRange.aspectMask = PixelFormatExtensions::IsDepthStencil(m_image->GetDesc().Format) ?
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
			VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = m_image->GetDesc().Levels;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = m_image->GetDesc().Layers;
		info.viewType = ToVulkanImageViewType(m_image->GetDesc().Type);
		info.format = ToVulkanFormat(m_image->GetDesc().Format);

		ThrowIfFailed(vkCreateImageView(m_device->Device, &info, nullptr, &m_vView));
		m_memoryUsage = 8;
		return true;
	}

	void GPUImageViewVulkan::OnReleaseGPU()
	{
		if (m_vView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_device->Device, m_vView, nullptr);
			m_memoryUsage = 0;
		}
	}

	void GPUImageViewVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Image_View, (u64)m_vView);
		}
	}

	GPUImageVulkan::GPUImageVulkan()
		: m_vImage(VK_NULL_HANDLE)
	{
	}

	GPUImageVulkan::~GPUImageVulkan()
	{
		OnReleaseGPU();
	}

	bool GPUImageVulkan::OnInit()
	{
		if (m_desc.IsEmpty())
		{
			return false;
		}

		VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
		imageCreateInfo.flags = ToVulkanImageCreateFlags(m_desc.CreateFlags);
		imageCreateInfo.imageType = ToVulkanImageType(m_desc.Type);
		imageCreateInfo.format = ToVulkanFormat(m_desc.Format);
		imageCreateInfo.extent = { m_desc.Width, m_desc.Height, m_desc.Depth };
		imageCreateInfo.mipLevels = m_desc.Levels;
		imageCreateInfo.arrayLayers = m_desc.Layers;
		imageCreateInfo.samples = ToVulkanSampleCountBits(m_desc.Samples);
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = ToVulkanImageUsage(m_desc.Usage);
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = m_device->GetQueueFamilyIndex(GPUQueue::GRAPHICS);
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.initialLayout = ToVulkanImageLayout(m_desc.InitLayout);

		VmaAllocationCreateInfo vmaAllocCreateInfo = {};
		vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		VmaAllocationInfo allocInfo;

		ThrowIfFailed(vmaCreateImage(m_device->VmaAllocator, &imageCreateInfo, &vmaAllocCreateInfo, &m_vImage, &m_vmaImageAlloc, &allocInfo));
		m_memoryUsage = allocInfo.size;

		if (m_desc.Data != nullptr)
		{
			GPUBufferVulkan stagingBuffer;
			stagingBuffer.Init(Insight::Graphics::GPUBufferDesc::RawUpload(m_desc.Size, m_desc.Data));

			GPUCommandBufferVulkan cmdBuffer;
			cmdBuffer.Init(Graphics::GPUCommandBufferDesc::CreateOneTimeCmdBuffer());
			cmdBuffer.BeginRecord();

			cmdBuffer.CopyBufferToImage(&stagingBuffer, this, &m_desc);

			cmdBuffer.EndRecord();
			cmdBuffer.SubmitAndWait(GPUQueue::GRAPHICS);
			cmdBuffer.ReleaseGPU();

			stbi_image_free(m_desc.Data);
			m_desc.Layout = ImageLayout::Shader_Read_Only;
		}

		return true;
	}

	void GPUImageVulkan::OnReleaseGPU()
	{
		if (m_vImage != VK_NULL_HANDLE)
		{
			vmaDestroyImage(m_device->VmaAllocator, m_vImage, m_vmaImageAlloc);
			m_memoryUsage = 0;
		}
	}

	void GPUImageVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Image, (u64)m_vImage);
		}
	}
}