#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Platforms/Platform.h"

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

				VkImageCreateInfo imageCreateInfo = {};
				imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageCreateInfo.imageType = TextureTypeToVulkan(m_infos.at(0).TextureType);
				imageCreateInfo.format = PixelFormatToVulkan(m_infos.at(0).Format);
				imageCreateInfo.extent = VkExtent3D { static_cast<u32>(m_infos.at(0).Width), static_cast<u32>(m_infos.at(0).Height), 1 };
				imageCreateInfo.mipLevels = m_infos.at(0).Mip_Count;
				imageCreateInfo.arrayLayers = m_infos.at(0).Layer_Count;
				imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageCreateInfo.usage = ImageUsageFlagsToVulkan(m_infos.at(0).ImageUsage);
				imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

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

				VkImageView imageView = CreateImageView(0, 1, m_infos.at(0).Layer_Count, 0);
				lock.lock();
				m_image_view = imageView;
				lock.unlock();

				// Create a image view for each layer. (Use image views when rendering to different layers).
				for (u32 i = 0; i < createInfo.Layer_Count; ++i)
				{
					VkImageView imageView = CreateImageView(0, 1, 1, i);
					lock.lock();
					m_single_layer_image_views.push_back(imageView);
					lock.unlock();
				}
				m_uploadStatus = createInfo.InitalStatus;
			}

			void RHI_Texture_Vulkan::Upload(void* data, int sizeInBytes)
			{
				IS_PROFILE_FUNCTION();
				ASSERT(Platform::IsMainThread());
				std::lock_guard lock(m_mutex);

				// We need a staging buffer to upload data from CPU to GPU.
				RHI_Buffer_Vulkan stagingBuffer;
				stagingBuffer.Create(m_context, BufferType::Staging, sizeInBytes, 0, { });
				stagingBuffer.Upload(data, sizeInBytes, 0, 0);

				RHI_CommandList* cmdList = m_context->GetCommandListManager().GetCommandList();
				m_uploadStatus = DeviceUploadStatus::Uploading;
				cmdList->CopyBufferToImage(this, &stagingBuffer);
				cmdList->Close();

				m_context->SubmitCommandListAndWait(cmdList);
				m_context->GetCommandListManager().ReturnCommandList(cmdList);
				stagingBuffer.Release();
				m_uploadStatus = DeviceUploadStatus::Completed;
			}

			std::vector<Byte> RHI_Texture_Vulkan::Download(void* data, int sizeInBytes)
			{
				IS_PROFILE_FUNCTION();
				ASSERT(Platform::IsMainThread());
				std::lock_guard lock(m_mutex);

				return std::vector<Byte>();
			}

			void RHI_Texture_Vulkan::Release()
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_mutex);

				for (u32 i = 0; i < m_single_layer_image_views.size(); ++i)
				{
					VkImageView& view = m_single_layer_image_views.at(i);
					if (view)
					{
						vkDestroyImageView(m_context->GetDevice(), view, nullptr);
						view = nullptr;
					}
				}
				m_single_layer_image_views.clear();

				if (m_image_view)
				{
					//if (m_info.Layer_Count > 1)
					{
						// We must have a multi layer image. Destroy the corresponding view which looks into all layers.
						vkDestroyImageView(m_context->GetDevice(), m_image_view, nullptr);
						m_image_view = nullptr;
					}
					m_image_view = nullptr;
				}

				if (m_image)
				{
					vmaDestroyImage(m_context->GetVMA(), m_image, m_imageAllocation);
					m_image = nullptr;
				}
			}

			bool RHI_Texture_Vulkan::ValidResource()
			{
				std::lock_guard lock(m_mutex);
				return m_image;
			}

			void RHI_Texture_Vulkan::SetName(std::string name)
			{
				std::lock_guard lock(m_mutex);

				if (m_image_view)
				{
					m_context->SetObjectName(name + "_Image_View", reinterpret_cast<u64>(m_image_view), VK_OBJECT_TYPE_IMAGE_VIEW);
				}

				for (u32 i = 0; i < m_single_layer_image_views.size(); ++i)
				{
					VkImageView& view = m_single_layer_image_views.at(i);
					if (view)
					{
						m_context->SetObjectName(name + "_Image_View_" + std::to_string(i), reinterpret_cast<u64>(view), VK_OBJECT_TYPE_IMAGE_VIEW);
					}
				}

				if (m_image)
				{
					m_context->SetObjectName(name, reinterpret_cast<u64>(m_image), VK_OBJECT_TYPE_IMAGE);
				}
				m_name = std::move(name);
			}

			VkImageView RHI_Texture_Vulkan::GetImageView() const
			{
				std::lock_guard lock(m_mutex);
				return m_image_view;
			}

			VkImageView RHI_Texture_Vulkan::GetImageView(u32 array_layer_index) const
			{
				std::lock_guard lock(m_mutex);
				return m_single_layer_image_views.at(array_layer_index);
			}

			VkImageView RHI_Texture_Vulkan::CreateImageView(u32 mip_index, u32 mip_count, u32 layer_count, u32 layer_index)
			{
				std::lock_guard lock(m_mutex);

				VkImageSubresourceRange imageSubresourceRange = {};
				imageSubresourceRange.aspectMask = m_infos.at(mip_index).ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment ?
					VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT : VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
				imageSubresourceRange.baseMipLevel = mip_index;
				imageSubresourceRange.levelCount = mip_count;
				imageSubresourceRange.baseArrayLayer = layer_index;
				imageSubresourceRange.layerCount = layer_count;

				VkImageViewCreateInfo viewCreateInfo = {};
				viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewCreateInfo.image = m_image;
				viewCreateInfo.viewType = TextureViewTypeToVulkan(m_infos.at(mip_index).TextureType);
				viewCreateInfo.format = PixelFormatToVulkan(m_infos.at(mip_index).Format);
				viewCreateInfo.components = VkComponentMapping();
				viewCreateInfo.subresourceRange = imageSubresourceRange;

				VkImageView imageView = nullptr;
				ThrowIfFailed(vkCreateImageView(m_context->GetDevice(), &viewCreateInfo, nullptr, &imageView));
				return imageView;
			}
		}
	}
}

#endif // defined(IS_VULKAN_ENABLED)