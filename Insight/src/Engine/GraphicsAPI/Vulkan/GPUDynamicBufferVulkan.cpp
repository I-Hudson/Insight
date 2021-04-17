#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDynamicBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"

namespace Insight::GraphicsAPI::Vulkan
{
	GPUDynamicBufferVulkan::GPUDynamicBufferVulkan()
	{ }

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
		
		VmaAllocationCreateInfo vmaInfo = { };
		vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		ThrowIfFailed(vmaCreateBuffer(m_device->VmaAllocator, &info, &vmaInfo, &m_buffer, &m_vmaAllocation, &m_vmaAllocationInfo));
		m_memoryUsage = m_desc.Size;
		vmaMapMemory(m_device->VmaAllocator, m_vmaAllocation, &m_mapped);
	}

	Graphics::GPUBuffer* GPUDynamicBufferVulkan::Upload(void* data, u64 size)
	{
		ASSERT(m_currentOffset + size <= m_desc.Size && "[GPUDynamicBufferVulkan::Upload] Dynamic buffer is too small.");

		u64 offset = m_currentOffset;
		char* target = (char*)m_mapped;
		target += m_currentOffset;
		Platform::MemCopy(target, data, size);
		m_currentOffset += size;
		m_currentOffset = PadData(m_currentOffset);

		Graphics::GPUBuffer* subBuffer = Graphics::GPUBuffer::New();
		subBuffer->Init(Graphics::GPUBufferDesc::SubAllocation(this, offset, size));
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
		vmaUnmapMemory(m_device->VmaAllocator, m_vmaAllocation);
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
}