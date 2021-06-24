
#include "Engine/GraphicsAPI/Vulkan/GPUBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDynamicBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"

namespace Insight::GraphicsAPI::Vulkan
{
	GPUBufferVulkan::GPUBufferVulkan()
		: m_buffer(nullptr)
	{ }

	GPUBufferVulkan::~GPUBufferVulkan()
	{
		ReleaseGPU();
	}

	void GPUBufferVulkan::Init(Graphics::GPUBufferDesc const& desc)
	{
		if (desc.SubAlloc.State == Graphics::GPUBufferSubAllocDesc::Unsed)
		{
			ASSERT(desc.Size > 0 && desc.Stride > 0);

			ReleaseGPU();
			m_desc = desc;

			VkBufferCreateInfo info = vks::initializers::bufferCreateInfo(ToVulkanBufferUsageFlags(m_desc.Flags), m_desc.Size);

			VmaAllocationCreateInfo vmaInfo = { };
			m_vmaMemoryUsage = ToVMAMemoryUsage(m_desc.Flags, m_mustBeMapped);
			vmaInfo.usage = m_vmaMemoryUsage;

			ThrowIfFailed(vmaCreateBuffer(m_device->VmaAllocator, &info, &vmaInfo, &m_buffer, &m_vmaAllocation, &m_vmaAllocationInfo));
			m_memoryUsage = m_desc.Size;

			m_bufferInfo.buffer = m_buffer;
			m_bufferInfo.range = m_desc.Size;
			m_bufferInfo.offset = 0;

			if (m_desc.InitData != nullptr)
			{
				SetData(m_desc.InitData, m_desc.Size);
			}
		}
		else
		{
			m_desc = desc;
			m_bufferInfo.buffer = static_cast<GPUDynamicBufferVulkan*>(m_desc.SubAlloc.ParentBuffer)->m_buffer;
			m_bufferInfo.range = m_desc.SubAlloc.Size;
			m_bufferInfo.offset = m_desc.SubAlloc.Begin;

			m_buffer = m_bufferInfo.buffer;
		}
	}

	void* GPUBufferVulkan::Map()
	{
		ThrowIfFailed(vmaMapMemory(m_device->VmaAllocator, m_vmaAllocation, &m_mappedData));
		return m_mappedData;
	}

	void GPUBufferVulkan::UnMap()
	{
		vmaUnmapMemory(m_device->VmaAllocator, m_vmaAllocation);
		m_mappedData = nullptr;
	}

	void GPUBufferVulkan::Upload(void* mapped, void const* data, u64 const& size)
	{
		if (mapped != nullptr)
		{
			Platform::MemCopy(mapped, data, size);
			// Try and flush. If the memory block is coherent then this will update automatically and will not be flushed VMA takes care of this. 
			ThrowIfFailed(vmaFlushAllocation(m_device->VmaAllocator, m_vmaAllocation, 0, size));
		}
		else
		{
			// Create staging buffer. 
			GPUBufferVulkan stagingBuffer;
			Graphics::GPUBufferDesc stagingBuffDesc = m_desc;
			stagingBuffDesc.Flags = Graphics::GPUBufferFlags::TRANSFER_SRC;
			stagingBuffer.Init(stagingBuffDesc);

			//Create command buffer
			GPUCommandBufferVulkan cmdBuffer;
			cmdBuffer.Init(Graphics::GPUCommandBufferDesc::CreateOneTimeCmdBuffer());
			cmdBuffer.BeginRecord();

			// Copy command
			cmdBuffer.CopyBuffer(&stagingBuffer, this, 1, 0, 0, m_desc.Size);

			// submit buffer 
			cmdBuffer.EndRecord();
			cmdBuffer.SubmitAndWait(GPUQueue::GRAPHICS);
			cmdBuffer.ReleaseGPU();
		}
	}

	void GPUBufferVulkan::Download(std::vector<u8>& data, void const* mapped)
	{
		if (mapped != nullptr)
		{
			Platform::MemCopy(data.data(), mapped, GetSize());
		}
		else
		{
			//ASSERT(m_desc.Flags & Graphics::GPUBufferFlags::TRANSFER_SRC && "[GPUBufferVulkan::Download] GPUBuffer must have 'GPUBufferFlags::TRANSFER_SRC' set or use a different VMA memory usage.");
			//TODO: This buffer needs to transitioned to be a source buffer.

			// Create staging buffer. 
			GPUBufferVulkan stagingBuffer;
			Graphics::GPUBufferDesc stagingBuffDesc = m_desc;
			stagingBuffDesc.Flags = Graphics::GPUBufferFlags::TRANSFER_DST;
			stagingBuffDesc.InitData = nullptr;
			stagingBuffer.Init(stagingBuffDesc);

			//Create command buffer
			GPUCommandBufferVulkan cmdBuffer;
			cmdBuffer.Init(Graphics::GPUCommandBufferDesc::CreateOneTimeCmdBuffer());
			cmdBuffer.BeginRecord();

			// Copy command
			cmdBuffer.CopyBuffer(this, &stagingBuffer, 1, 0, 0, m_desc.Size);

			// submit buffer 
			cmdBuffer.EndRecord();
			cmdBuffer.SubmitAndWait(GPUQueue::GRAPHICS);
			cmdBuffer.ReleaseGPU();

			stagingBuffer.GetData(data);
		}
	}

	void GPUBufferVulkan::OnReleaseGPU()
	{
		vmaDestroyBuffer(m_device->VmaAllocator, m_buffer, m_vmaAllocation);
	}

	void GPUBufferVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Buffer, (u64)m_buffer);
		}
	}
}

