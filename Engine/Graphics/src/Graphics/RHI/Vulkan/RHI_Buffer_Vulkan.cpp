#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Platforms/Platform.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"


#include "VmaUsage.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			RHI_Buffer_Vulkan::~RHI_Buffer_Vulkan()
			{
				Release();
			}

			void RHI_Buffer_Vulkan::Create(RenderContext* context, BufferType bufferType, u64 sizeBytes, u64 stride, RHI_Buffer_Overrides overrides)
			{
				std::lock_guard lock(m_mutex);

				m_context = static_cast<RenderContext_Vulkan*>(context);
				m_bufferType = bufferType;
				m_size = sizeBytes;
				m_stride = stride;
				m_overrides = overrides;

				VkBufferCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				createInfo.size = m_size;
				createInfo.usage = BufferTypeToVulkanBufferUsageFlags(bufferType);

				VmaAllocationCreateInfo vmaInfo = { };
				vmaInfo.flags = BufferTypeToVMAAllocCreateFlags(m_bufferType);
				vmaInfo.usage = BufferTypeToVMAUsage(m_bufferType);

				if (overrides.Force_Host_Writeable)
				{
					vmaInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
					vmaInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
				}

				ThrowIfFailed(vmaCreateBuffer(m_context->GetVMA(), 
					&createInfo, 
					&vmaInfo, 
					reinterpret_cast<VkBuffer*>(&m_buffer), 
					&m_vmaAllocation, nullptr));

				if (vmaInfo.usage == VMA_MEMORY_USAGE_AUTO_PREFER_HOST 
					|| vmaInfo.usage == VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
				{
					vmaMapMemory(m_context->GetVMA(), m_vmaAllocation, &m_mappedData);
					m_uploadStatus = DeviceUploadStatus::Completed;
				}
			}

			RHI_BufferView RHI_Buffer_Vulkan::Upload(const void* data, u64 sizeInBytes, u64 offset, u64 alignment)
			{
				IS_PROFILE_FUNCTION();
				ASSERT(Platform::IsMainThread());
				std::lock_guard lock(m_mutex);

				if (data == nullptr)
				{
					return {};
				}

				if (sizeInBytes > GetSize())
				{
					IS_CORE_ERROR("[RHI_Buffer_Vulkan::Upload] Upload size '{}' is too big avaliable size '{}'.", sizeInBytes, GetSize());
					return {};
				}

				m_uploadStatus = DeviceUploadStatus::Uploading;
				if (m_mappedData)
				{
					IS_PROFILE_SCOPE("Memcpy");
					Platform::MemCopy((Byte*)m_mappedData + offset, data, sizeInBytes);
				}
				else
				{
					IS_PROFILE_SCOPE("Staging");
					/// We need a staging buffer to upload data from CPU to GPU.
					RHI_Buffer_Vulkan stagingBuffer;
					stagingBuffer.Create(m_context, BufferType::Staging, sizeInBytes, 0, { });
					stagingBuffer.Upload(data, sizeInBytes, 0, 0);

					RHI_CommandList* cmdList = m_context->GetCommandListManager().GetCommandList();
					cmdList->CopyBufferToBuffer(this, offset, &stagingBuffer, 0, sizeInBytes);
					cmdList->Close();

					m_context->SubmitCommandListAndWait(cmdList);
					m_context->GetCommandListManager().ReturnCommandList(cmdList);

					stagingBuffer.Release();
				}
				sizeInBytes = AlignUp(sizeInBytes, alignment);
				m_uploadStatus = DeviceUploadStatus::Completed;
				return RHI_BufferView(this, offset, sizeInBytes);
			}

			std::vector<Byte> RHI_Buffer_Vulkan::Download()
			{
				IS_PROFILE_FUNCTION();
				//ASSERT(Platform::IsMainThread());
				std::lock_guard lock(m_mutex);

				const u64 current_buffer_size = GetSize();
				std::vector<Byte> data;
				data.resize(current_buffer_size);

				if (m_mappedData)
				{
					Platform::MemCopy((Byte*)data.data(), m_mappedData, current_buffer_size);
				}
				else
				{
					/// We need a staging buffer to upload data from CPU to GPU.
					RHI_Buffer_Vulkan readback_buffer;
					readback_buffer.Create(m_context, BufferType::Readback, current_buffer_size, GetStride(), { });

					RHI_CommandList* cmdList = m_context->GetCommandListManager().GetCommandList();
					cmdList->CopyBufferToBuffer(&readback_buffer, 0, this, 0, GetSize());
					cmdList->Close();

					m_context->SubmitCommandListAndWait(cmdList);
					m_context->GetCommandListManager().ReturnCommandList(cmdList);

					data = readback_buffer.Download();

					readback_buffer.Release();
				}
				return data;
			}

			void RHI_Buffer_Vulkan::Resize(u64 newSizeBytes)
			{
				std::vector<Byte> data = Download();
				const u64 data_size = GetSize();
				
				Release();
				Create(m_context, m_bufferType, newSizeBytes, m_stride, m_overrides);

				Upload(data.data(), data_size, 0, 0);
			}

			void RHI_Buffer_Vulkan::Release()
			{
				std::lock_guard lock(m_mutex);
				if (m_buffer)
				{
					if (m_mappedData)
					{
						vmaUnmapMemory(m_context->GetVMA(), m_vmaAllocation);
					}

					vmaDestroyBuffer(m_context->GetVMA(), m_buffer, m_vmaAllocation);
					m_buffer = nullptr;
				}
				m_size = 0;
			}

			bool RHI_Buffer_Vulkan::ValidResource()
			{
				return m_buffer;
			}

			void RHI_Buffer_Vulkan::SetName(std::string name)
			{
				m_context->SetObjectName(name, (u64)m_buffer, VK_OBJECT_TYPE_BUFFER);
				m_name = std::move(name);
			}
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)