#include "Graphics/GPU/RHI/Vulkan/GPUBuffer_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/GPU/GPUCommandList.h"
#include "Core/Logger.h"

#include "VmaUsage.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			GPUBuffer_Vulkan::GPUBuffer_Vulkan()
			{
			}

			GPUBuffer_Vulkan::~GPUBuffer_Vulkan()
			{
				Destroy();
			}

			void GPUBuffer_Vulkan::Create(GPUBufferCreateInfo desc)
			{
				m_info = desc;
				if (m_info.Type == GPUBufferType::Invalid)
				{
					IS_CORE_ERROR("[GPUBuffer_Vulkan::Create] Buffer type is invalid.");
					exit(24);
				}

				m_info = desc;
				
				vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo({}, desc.Size, GPUBufferTypeToVulkanBufferUsageFlags(desc.Type));
				VmaAllocationCreateInfo vmaInfo = { };
				vmaInfo.flags = m_info.Type == GPUBufferType::Staging || m_info.Type == GPUBufferType::Readback ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0;
				vmaInfo.usage = GPUBufferTypeToVMAUsage(m_info.Type);

				VkResult result = vmaCreateBuffer(GetDevice()->GetVMAAllocator(), 
					reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo),
					&vmaInfo,
					reinterpret_cast<VkBuffer*>(&m_buffer), 
					&m_vmaAllocation, nullptr);
			}

			void GPUBuffer_Vulkan::Destroy()
			{
				if (m_buffer)
				{
					vmaDestroyBuffer(GetDevice()->GetVMAAllocator(), m_buffer, m_vmaAllocation);
					m_buffer = nullptr;
				}
			}

			bool GPUBuffer_Vulkan::Upload(const void* data, u64 size, u64 offset)
			{
				if (m_info.Type == GPUBufferType::Staging)
				{
					void* map = nullptr;
					VkResult result = vmaMapMemory(GetDevice()->GetVMAAllocator(), m_vmaAllocation, &map);
					if (map != nullptr)
					{
						uintptr_t dst = (uintptr_t)map + offset;
						memcpy((void*)dst, data, size);
						vmaFlushAllocation(GetDevice()->GetVMAAllocator(), m_vmaAllocation, offset, size);
						vmaUnmapMemory(GetDevice()->GetVMAAllocator(), m_vmaAllocation);
					}
				}
				else
				{
					constexpr const char* CMD_GPU_BUFFER_VULKAN = "CMD_GPU_BUFFER_VULKAN";
					GPUCommandList* cmdList = GPUCommandListManager::Instance().GetOrCreateCommandList(CMD_GPU_BUFFER_VULKAN, GPUCommandListType::Transient);
					GPUBuffer* stagingBuffer = GPUBufferManager::Instance().CreateBuffer("StagingBuffer", GPUBufferCreateInfo(m_info.Size, GPUBufferType::Staging));
					stagingBuffer->SetData(data, size, offset);

					cmdList->BeginRecord();
					cmdList->CopyBufferToBuffer(stagingBuffer, this);
					cmdList->EndRecord();
					cmdList->SubmitAndWait(GPUQueue_Graphics);

					GPUBufferManager::Instance().DestroyBuffer(stagingBuffer);
					GPUCommandListManager::Instance().ReturnCommandList(CMD_GPU_BUFFER_VULKAN, cmdList);
				}

				return true;
			}
		}
	}
}