#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDynamicBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"

namespace Insight::GraphicsAPI::Vulkan
{
	GPUDynamicBufferVulkan::GPUDynamicBufferVulkan()
	{
		m_mapped = nullptr;
	}

	GPUDynamicBufferVulkan::~GPUDynamicBufferVulkan()
	{
		ReleaseGPU();
	}

	void GPUDynamicBufferVulkan::Init(Graphics::GPUDynamicBufferDesc & desc)
	{
		ASSERT(desc.Size > 0 && "[GPUDynamicBufferVulkan::Init] Buffer size must be greater than 0.");
		ASSERT(desc.Size != m_desc.Size && "[GPUDynamicBufferVulkan::Init] Buffer size is the same. Use 'Reset'.");

		ReleaseGPU();

		m_desc = desc;
		Reset();

		VkBufferCreateInfo info = vks::initializers::bufferCreateInfo(ToVulkanBufferUsageFlags(m_desc.Flags), m_desc.Size);
		
		bool mustBeMapped = false;
		VmaAllocationCreateInfo vmaInfo = { };
		vmaInfo.usage = ToVMAMemoryUsage(desc.Flags, mustBeMapped);

		ThrowIfFailed(vmaCreateBuffer(m_device->VmaAllocator, &info, &vmaInfo, &m_buffer, &m_vmaAllocation, &m_vmaAllocationInfo));
		m_memoryUsage = m_desc.Size;
		if (mustBeMapped)
		{
			ThrowIfFailed(vmaMapMemory(m_device->VmaAllocator, m_vmaAllocation, &m_mapped));
		}
	}

	Graphics::GPUBuffer* GPUDynamicBufferVulkan::Upload(void* data, u64 size)
	{
		//ASSERT(m_currentOffset + size <= m_desc.Size && "[GPUDynamicBufferVulkan::Upload] Dynamic buffer is too small.");
		if (m_currentOffset + size > m_desc.Size)
		{
			IS_CORE_ERROR("[GPUDynamicBufferVulkan::Upload] Dynamic buffer is too small. Dynamic buffer is being resized.");
			Resize();
		}

		u64 offset = m_currentOffset;
		if (m_mapped)
		{
			char* target = (char*)m_mapped;
			target += m_currentOffset;
			Platform::MemCopy(target, data, size);
		}
		else
		{
			// Create staging buffer. 
			GPUBufferVulkan stagingBuffer;
			Graphics::GPUBufferDesc stagingBuffDesc = Graphics::GPUBufferDesc();
			stagingBuffDesc.Flags = m_desc.Flags | Graphics::GPUBufferFlags::TRANSFER_SRC;
			stagingBuffDesc.Size = size;
			stagingBuffDesc.Stride = 1;
			stagingBuffDesc.InitData = data;
			stagingBuffer.Init(stagingBuffDesc);

			//Create command buffer
			GPUCommandBufferVulkan cmdBuffer;
			cmdBuffer.Init(Graphics::GPUCommandBufferDesc::CreateOneTimeCmdBuffer());
			cmdBuffer.BeginRecord();

			// Copy command
			cmdBuffer.CopyBufferToDynamic(&stagingBuffer, this, 1, 0, m_currentOffset, size);

			// submit buffer 
			cmdBuffer.EndRecord();
			cmdBuffer.SubmitAndWait(GPUQueue::GRAPHICS);
			cmdBuffer.ReleaseGPU();
		}
		m_currentOffset += size;
		m_currentOffset = PadData(m_currentOffset);

		Graphics::GPUBuffer* subBuffer = Graphics::GPUBuffer::New();
		subBuffer->Init(Graphics::GPUBufferDesc::SubAllocation(this, offset, size, m_desc.Flags));
		m_subBuffers.push_back(subBuffer);
		return subBuffer;
	}

	void GPUDynamicBufferVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Buffer, (u64)m_buffer);
		}
	}

	void GPUDynamicBufferVulkan::OnReleaseGPU()
	{
		if (m_mapped)
		{
			vmaUnmapMemory(m_device->VmaAllocator, m_vmaAllocation);
		}
		vmaDestroyBuffer(m_device->VmaAllocator, m_buffer, m_vmaAllocation);
	}

	u64 GPUDynamicBufferVulkan::PadData(u64 orginalSize)
	{
		u64 minSize = m_desc.AligmentSize;
		u64 alignedSize = orginalSize;
		if (minSize > 0)
		{
			alignedSize = (alignedSize + minSize - 1) & ~(minSize - 1);
		}
		return alignedSize;
	}

	void GPUDynamicBufferVulkan::Resize()
	{
		// Double the buffer size and create a new buffer.
		m_desc.Size *= 2;
		VkBufferCreateInfo info = vks::initializers::bufferCreateInfo(ToVulkanBufferUsageFlags(m_desc.Flags), m_desc.Size);

		bool mustBeMapped = false;
		VmaAllocationCreateInfo vmaInfo = { };
		vmaInfo.usage = ToVMAMemoryUsage(m_desc.Flags, mustBeMapped);

		VkBuffer newBuffer;
		VmaAllocation newVmaAllocation;
		VmaAllocationInfo newVmaAllocationInfo;
		ThrowIfFailed(vmaCreateBuffer(m_device->VmaAllocator, &info, &vmaInfo, &newBuffer, &newVmaAllocation, &newVmaAllocationInfo));

		m_memoryUsage = m_desc.Size;
		void* newMapped = nullptr;
		if (mustBeMapped)
		{
			ThrowIfFailed(vmaMapMemory(m_device->VmaAllocator, m_vmaAllocation, &newMapped));
		}

		// Copy all contents to the new buffer
		if (newMapped)
		{
			char* target = (char*)newMapped;
			target += m_currentOffset;
			Platform::MemCopy(target, m_mapped, m_currentOffset);
		}
		else
		{
			//Create command buffer
			GPUCommandBufferVulkan cmdBuffer;
			cmdBuffer.Init(Graphics::GPUCommandBufferDesc::CreateOneTimeCmdBuffer());
			cmdBuffer.BeginRecord();

			// Copy command
			VkBufferCopy copy = { };
			copy.srcOffset = 0;
			copy.dstOffset = 0;
			copy.size = m_currentOffset;
			vkCmdCopyBuffer(cmdBuffer.GetCmdBuffer(), m_buffer, newBuffer, 1, &copy);

			// submit buffer 
			cmdBuffer.EndRecord();
			cmdBuffer.SubmitAndWait(GPUQueue::GRAPHICS);
			cmdBuffer.ReleaseGPU();
		}

		// Release the old buffer and destroy.
		ReleaseGPU();

		// Set the new buffer to this.
		m_buffer = newBuffer;
		m_vmaAllocation = newVmaAllocation;
		m_vmaAllocationInfo = newVmaAllocationInfo;
	}
}