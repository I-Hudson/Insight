#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Platform/Platform.h"

#include "Core/Profiler.h"

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

			void RHI_Texture_Vulkan::Create(RenderContext* context, RHI_TextureInfo createInfo)
			{
				IS_PROFILE_FUNCTION();
				std::unique_lock lock(m_mutex);

				m_context = static_cast<RenderContext_Vulkan*>(context);
				for (size_t i = 0; i < createInfo.Mip_Count; ++i)
				{
					m_infos.push_back(createInfo);
				}

				vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo(
					{},
					TextureTypeToVulkan(m_infos.at(0).TextureType),
					PixelFormatToVulkan(m_infos.at(0).Format),
					vk::Extent3D(m_infos.at(0).Width, m_infos.at(0).Height, 1),
					m_infos.at(0).Mip_Count,								// mip levels 
					m_infos.at(0).Layer_Count,								// array layers
					vk::SampleCountFlagBits::e1,
					vk::ImageTiling::eOptimal,
					ImageUsageFlagsToVulkan(m_infos.at(0).ImageUsage),
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
				lock.unlock();

				vk::ImageView imageView = CreateImageView(0, 1, m_infos.at(0).Layer_Count, 0);
				lock.lock();
				m_image_view = imageView;
				lock.unlock();

				// Create a image view for each layer. (Use image views when rendering to different layers).
				for (u32 i = 0; i < createInfo.Layer_Count; ++i)
				{
					vk::ImageView imageView = CreateImageView(0, 1, 1, i);
					lock.lock();
					m_single_layer_image_views.push_back(imageView);
					lock.unlock();
				}
			}

			void RHI_Texture_Vulkan::Upload(void* data, int sizeInBytes)
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_mutex);

				/// We need a staging buffer to upload data from CPU to GPU.
				RHI_Buffer_Vulkan stagingBuffer;
				stagingBuffer.Create(m_context, BufferType::Staging, sizeInBytes, 0, { });
				stagingBuffer.Upload(data, sizeInBytes, 0);

				RHI_CommandList* cmdList = m_context->GetCommandListManager().GetCommandList();
				cmdList->CopyBufferToImage(this, &stagingBuffer);
				cmdList->Close();

				m_context->SubmitCommandListAndWait(cmdList);
				m_context->GetCommandListManager().ReturnCommandList(cmdList);

				stagingBuffer.Release();
			}

			std::vector<Byte> RHI_Texture_Vulkan::Download(void* data, int sizeInBytes)
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_mutex);

				return std::vector<Byte>();
			}

			void RHI_Texture_Vulkan::Release()
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_mutex);

				for (u32 i = 0; i < m_single_layer_image_views.size(); ++i)
				{
					vk::ImageView& view = m_single_layer_image_views.at(i);
					if (view)
					{
						m_context->GetDevice().destroyImageView(view);
					}
				}
				m_single_layer_image_views.clear();

				if (m_image_view)
				{
					//if (m_info.Layer_Count > 1)
					{
						// We must have a multi layer image. Destroy the corresponding view which looks into all layers.
						m_context->GetDevice().destroyImageView(m_image_view);
					}
					m_image_view = nullptr;
				}

				if (m_image)
				{
					vmaDestroyImage(m_context->GetVMA(), m_image, m_imageAllocation);
					m_image = nullptr;
				}
			}

			bool RHI_Texture_Vulkan::ValidResouce()
			{
				std::lock_guard lock(m_mutex);
				return m_image;
			}

			void RHI_Texture_Vulkan::SetName(std::wstring name)
			{
				std::lock_guard lock(m_mutex);

				if (m_image_view)
				{
					m_context->SetObejctName(name + L"_Image_View", reinterpret_cast<u64>(m_image_view.operator VkImageView()), vk::ObjectType::eImageView);
				}

				for (u32 i = 0; i < m_single_layer_image_views.size(); ++i)
				{
					vk::ImageView& view = m_single_layer_image_views.at(i);
					if (view)
					{
						m_context->SetObejctName(name + L"_Image_View_" + std::to_wstring(i), reinterpret_cast<u64>(view.operator VkImageView()), vk::ObjectType::eImageView);
					}
				}

				if (m_image)
				{
					m_context->SetObejctName(name, reinterpret_cast<u64>(m_image.operator VkImage()), vk::ObjectType::eImage);
				}
				m_name = std::move(name);
			}

			vk::ImageView RHI_Texture_Vulkan::GetImageView() const
			{
				std::lock_guard lock(m_mutex);
				return m_image_view;
			}

			vk::ImageView RHI_Texture_Vulkan::GetImageView(u32 array_layer_index) const
			{
				std::lock_guard lock(m_mutex);
				return m_single_layer_image_views.at(array_layer_index);
			}

			vk::ImageView RHI_Texture_Vulkan::CreateImageView(u32 mip_index, u32 mip_count, u32 layer_count, u32 layer_index)
			{
				std::lock_guard lock(m_mutex);

				vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo(
					{ },
					m_image,
					TextureViewTypeToVulkan(m_infos.at(mip_index).TextureType),
					PixelFormatToVulkan(m_infos.at(mip_index).Format),
					vk::ComponentMapping(),
					vk::ImageSubresourceRange(
						m_infos.at(mip_index).ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment ?
						vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor,
						mip_index,
						mip_count,
						layer_index,
						layer_count)
				);
				return m_context->GetDevice().createImageView(viewCreateInfo);
			}
		}
	}
}

#endif // defined(IS_VULKAN_ENABLED)