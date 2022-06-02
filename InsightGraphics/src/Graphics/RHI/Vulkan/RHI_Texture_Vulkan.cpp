#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Platform/Platform.h"

#include "VmaUsage.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			RHI_Texture_Vulkan::~RHI_Texture_Vulkan()
			{
				Release();
			}

			void RHI_Texture_Vulkan::Create(RenderContext* context, RHI_TextureCreateInfo createInfo)
			{
				m_context = dynamic_cast<RenderContext_Vulkan*>(context);
				m_info = createInfo;

				vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo(
					{},
					vk::ImageType::e2D,
					PixelFormatToVulkan(m_info.Format),
					vk::Extent3D(m_info.Width, m_info.Height, 1),
					1,	// mip levels 
					1,	// array layers
					vk::SampleCountFlagBits::e1,
					vk::ImageTiling::eOptimal,
					ImageUsageFlagsToVulkan(m_info.ImageUsage),
					vk::SharingMode::eExclusive
				);

				VmaAllocationCreateInfo vmaAllocCreateInfo = {};
				vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
				VmaAllocationInfo allocInfo;

				ThrowIfFailed(vmaCreateImage(m_context->GetVMA(),
					reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo),
					&vmaAllocCreateInfo,
					reinterpret_cast<VkImage*>(&m_image),
					&m_imageAllocation,
					&allocInfo));

				vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo(
					{ },
					m_image,
					vk::ImageViewType::e2D,
					imageCreateInfo.format,
					vk::ComponentMapping(),
					vk::ImageSubresourceRange(
						createInfo.ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment ?
						vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor,
						0,
						1,
						0,
						1)
				);
				m_imageView = m_context->GetDevice().createImageView(viewCreateInfo);
			}

			void RHI_Texture_Vulkan::Upload(void* data, int sizeInBytes)
			{
				// We need a staging buffer to upload data from CPU to GPU.
				RHI_Buffer_Vulkan stagingBuffer;
				stagingBuffer.Create(m_context, BufferType::Staging, sizeInBytes, 0);
				stagingBuffer.Upload(data, sizeInBytes, 0);

				RHI_CommandList* cmdList = m_context->GetFrameResouce().CommandListManager.GetCommandList();
				cmdList->CopyBufferToImage(this, &stagingBuffer);
				cmdList->Close();

				m_context->SubmitCommandListAndWait(cmdList);
				m_context->GetFrameResouce().CommandListManager.ReturnCommandList(cmdList);

				stagingBuffer.Release();
			}

			std::vector<Byte> RHI_Texture_Vulkan::Download(void* data, int sizeInBytes)
			{
				return std::vector<Byte>();
			}

			void RHI_Texture_Vulkan::Release()
			{
				if (m_imageView)
				{
					m_context->GetDevice().destroyImageView(m_imageView);
					m_imageView = nullptr;
				}

				if (m_image)
				{
					vmaDestroyImage(m_context->GetVMA(), m_image, m_imageAllocation);
					m_image = nullptr;
				}
			}

			bool RHI_Texture_Vulkan::ValidResouce()
			{
				return m_image;
			}

			void RHI_Texture_Vulkan::SetName(std::wstring name)
			{
				if (m_imageView)
				{
					m_context->SetObejctName(name, reinterpret_cast<u64>(m_imageView.operator VkImageView()), vk::ObjectType::eImageView);
				}

				if (m_image)
				{
					m_context->SetObejctName(name, reinterpret_cast<u64>(m_image.operator VkImage()), vk::ObjectType::eImage);
				}
			}
		}
	}
}

#endif // defined(IS_VULKAN_ENABLED)