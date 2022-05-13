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

			void RHI_Buffer_Vulkan::Create(RenderContext* context, BufferType bufferType, u64 sizeBytes)
			{
				m_context = dynamic_cast<RenderContext_Vulkan*>(context);
				m_bufferType = bufferType;
				m_size = sizeBytes;

				vk::BufferCreateInfo createInfo = {};
				createInfo.setSize(m_size);
				createInfo.setUsage(BufferTypeToVulkanBufferUsageFlags(bufferType));

				VmaAllocationCreateInfo vmaInfo = { };
				vmaInfo.flags = BufferTypeToVMAAllocCreateFlags(m_bufferType);
				vmaInfo.usage = BufferTypeToVMAUsage(m_bufferType);

				ThrowIfFailed(vmaCreateBuffer(m_context->GetVMA(), 
					reinterpret_cast<const VkBufferCreateInfo*>(&createInfo), 
					&vmaInfo, 
					reinterpret_cast<VkBuffer*>(&m_buffer), 
					&m_vmaAllocation, nullptr));

				if (m_bufferType == BufferType::Uniform
					|| m_bufferType == BufferType::Storage
					|| m_bufferType == BufferType::Raw
					|| m_bufferType == BufferType::Staging
					|| m_bufferType == BufferType::Readback)
				{
					vmaMapMemory(m_context->GetVMA(), m_vmaAllocation, &m_mappedData);
				}
			}

			RHI_BufferView RHI_Buffer_Vulkan::Upload(void* data, int sizeInBytes, int offset)
			{
				if (m_mappedData)
				{
					Platform::MemCopy((Byte*)m_mappedData + offset, data, sizeInBytes);
				}
				else
				{
					// We need a staging buffer to upload data from CPU to GPU.
					RHI_Buffer_Vulkan stagingBuffer;
					stagingBuffer.Create(m_context, BufferType::Staging, sizeInBytes);
					stagingBuffer.Upload(data, sizeInBytes, offset);

					RHI_CommandList* cmdList = m_context->GetFrameResouce().CommandListManager.GetCommandList();
					cmdList->CopyBufferToBuffer(this, &stagingBuffer);
					cmdList->Close();

					m_context->SubmitCommandListAndWait(cmdList);
					m_context->GetFrameResouce().CommandListManager.ReturnCommandList(cmdList);

					stagingBuffer.Release();
				}
				return RHI_BufferView(this, offset, sizeInBytes);
			}

			std::vector<Byte> RHI_Buffer_Vulkan::Download()
			{
				return std::vector<Byte>();
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
					Create(m_context, m_bufferType, newSizeInBytes);
					Upload(data.data(), (int)data.size(), 0);
				}
			}
		}
	}
}