#include "Graphics/GPU/RHI/Vulkan/GPUImage_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/PixelFormatExtensions.h"

#include "VmaUsage.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			GPUImage_Vulkan::~GPUImage_Vulkan()
			{
				Destroy();
			}

			void GPUImage_Vulkan::LoadFromData(GPUImageCreateInfo info)
			{
				vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo(
					{},
					vk::ImageType::e2D,
					PixelFormatToVkFormat[(int)info.Format],
					{static_cast<u32>(m_width), static_cast<u32>(m_height), 1Ui32},
					1,
					1,
					vk::SampleCountFlagBits::e1,
					vk::ImageTiling::eOptimal,
					ImageUsageFlagsToVulkan(info.Usage),
					vk::SharingMode::eExclusive,
					{ },
					vk::ImageLayout::eUndefined
					);

				VmaAllocationCreateInfo vmaAllocCreateInfo = {};
				vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
				VmaAllocationInfo allocInfo;
				vmaCreateImage(GetDevice()->GetVMAAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &vmaAllocCreateInfo, reinterpret_cast<VkImage*>(&m_image), &m_vmaAllocation, &allocInfo);

				vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo(
					{ },
					m_image,
					vk::ImageViewType::e2D,
					imageCreateInfo.format
				);
				viewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(
					PixelFormatExtensions::IsDepth(info.Format) ? vk::ImageAspectFlagBits::eDepth :
					PixelFormatExtensions::IsDepthStencil(info.Format) ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eColor,
					0, 
					1,
					0,
					1
					));
				m_view = GetDevice()->GetDevice().createImageView(viewCreateInfo);


				if (!info.Data.empty())
				{
					// Get buffer, make is a staging buffer
					// upload data
					// CopyBufferToImage
					// SubmitAnd Wait
					// Release everything
				}
			}

			void GPUImage_Vulkan::Destroy()
			{
				m_width = 0;
				m_height = 0;
				m_channels = 0;
				m_loadedFromFile = false;
				m_fileName.resize(0);

				if (m_image)
				{
					vmaDestroyImage(GetDevice()->GetVMAAllocator(), *reinterpret_cast<VkImage*>(&m_image), m_vmaAllocation);
					m_image = vk::Image(nullptr);
				}
				if (m_view)
				{
					GetDevice()->GetDevice().destroyImageView(m_view);
					m_view = vk::ImageView(nullptr);
				}
			}
		}
	}
}