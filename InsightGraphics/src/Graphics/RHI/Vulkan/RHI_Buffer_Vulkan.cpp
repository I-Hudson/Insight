#if defined(IS_VULKAN_ENABLED)

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
			RHI_Buffer_Vulkan::~RHI_Buffer_Vulkan()
			{
				Release();
			}

			void RHI_Buffer_Vulkan::Create(RenderContext* context, BufferType bufferType, u64 sizeBytes, int stride, RHI_Buffer_Overrides overrides)
			{
				m_context = static_cast<RenderContext_Vulkan*>(context);
				m_bufferType = bufferType;
				m_size = sizeBytes;
				m_stride = stride;
				m_overrides = overrides;

				vk::BufferCreateInfo createInfo = {};
				createInfo.setSize(m_size);
				createInfo.setUsage(BufferTypeToVulkanBufferUsageFlags(bufferType));

				VmaAllocationCreateInfo vmaInfo = { };
				vmaInfo.flags = BufferTypeToVMAAllocCreateFlags(m_bufferType);
				vmaInfo.usage = BufferTypeToVMAUsage(m_bufferType);

				if (overrides.Force_Host_Writeable)
				{
					vmaInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
					vmaInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
				}

				ThrowIfFailed(vmaCreateBuffer(m_context->GetVMA(), 
					reinterpret_cast<const VkBufferCreateInfo*>(&createInfo), 
					&vmaInfo, 
					reinterpret_cast<VkBuffer*>(&m_buffer), 
					&m_vmaAllocation, nullptr));

				if (vmaInfo.usage == VMA_MEMORY_USAGE_AUTO_PREFER_HOST 
					|| vmaInfo.usage == VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
				{
					vmaMapMemory(m_context->GetVMA(), m_vmaAllocation, &m_mappedData);
				}
			}

			RHI_BufferView RHI_Buffer_Vulkan::Upload(const void* data, int sizeInBytes, int offset)
			{
				IS_PROFILE_FUNCTION();

				if (m_mappedData)
				{
					Platform::MemCopy((Byte*)m_mappedData + offset, data, sizeInBytes);
				}
				else
				{
					// We need a staging buffer to upload data from CPU to GPU.
					RHI_Buffer_Vulkan stagingBuffer;
					stagingBuffer.Create(m_context, BufferType::Staging, sizeInBytes, 0, { });
					stagingBuffer.Upload(data, sizeInBytes, 0);

					RHI_CommandList* cmdList = m_context->GetCommandListManager().GetCommandList();
					cmdList->CopyBufferToBuffer(this, &stagingBuffer, offset);
					cmdList->Close();

					m_context->SubmitCommandListAndWait(cmdList);
					m_context->GetCommandListManager().ReturnCommandList(cmdList);

					stagingBuffer.Release();
				}
				return RHI_BufferView(this, offset, sizeInBytes);
			}

			std::vector<Byte> RHI_Buffer_Vulkan::Download()
			{
				const u64 current_buffer_size = GetSize();
				std::vector<Byte> data;
				data.resize(current_buffer_size);

				if (m_mappedData)
				{
					Platform::MemCopy((Byte*)data.data(), m_mappedData, current_buffer_size);
				}
				else
				{
					// We need a staging buffer to upload data from CPU to GPU.
					RHI_Buffer_Vulkan readback_buffer;
					readback_buffer.Create(m_context, BufferType::Readback, current_buffer_size, GetStride(), { });

					RHI_CommandList* cmdList = m_context->GetCommandListManager().GetCommandList();
					cmdList->CopyBufferToBuffer(&readback_buffer, this, 0);
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

				Upload(data.data(), data_size, 0);
			}

			void RHI_Buffer_Vulkan::Release()
			{
				if (m_buffer)
				{
					if (m_mappedData)
					{
						vmaUnmapMemory(m_context->GetVMA(), m_vmaAllocation);
					}

					vmaDestroyBuffer(m_context->GetVMA(), m_buffer, m_vmaAllocation);
					m_buffer = nullptr;
				}
			}

			bool RHI_Buffer_Vulkan::ValidResouce()
			{
				return m_buffer;
			}

			void RHI_Buffer_Vulkan::SetName(std::wstring name)
			{
				m_context->SetObejctName(name, (u64)m_buffer.operator VkBuffer(), vk::Buffer::objectType);
			}

			void RHI_Buffer_Vulkan::Resize(int newSizeInBytes)
			{
				if (m_buffer && m_size < newSizeInBytes)
				{
					std::vector<Byte> data = Download();
					Release();
					Create(m_context, m_bufferType, newSizeInBytes, (int)m_stride, { });
					Upload(data.data(), (int)data.size(), 0);
				}
			}
		}
	}
}

#endif //#if defined(IS_VULKAN_ENABLED)