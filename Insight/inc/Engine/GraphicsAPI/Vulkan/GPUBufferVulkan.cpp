#include "ispch.h"
#include "GPUBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"

namespace Insight::GraphicsAPI::Vulkan
{
	VmaMemoryUsage ToVMAMemoryUsage(Graphics::GPUBufferFlags const& flags)
	{
		VmaMemoryUsage usage = VMA_MEMORY_USAGE_UNKNOWN;
		if (flags & Graphics::GPUBufferFlags::TRANSFER_SRC) { usage = VMA_MEMORY_USAGE_CPU_ONLY; }
		if (flags & Graphics::GPUBufferFlags::TRANSFER_DST) { usage = VMA_MEMORY_USAGE_GPU_TO_CPU; }

		if (flags & Graphics::GPUBufferFlags::UNIFORM) { usage = VMA_MEMORY_USAGE_CPU_TO_GPU; }

		if (flags & Graphics::GPUBufferFlags::VERTEX) { usage = VMA_MEMORY_USAGE_GPU_ONLY; }
		if (flags & Graphics::GPUBufferFlags::INDEX) { usage = VMA_MEMORY_USAGE_GPU_ONLY; }

		if (flags & Graphics::GPUBufferFlags::VERTEX && flags & Graphics::GPUBufferFlags::TRANSFER_SRC) { usage = VMA_MEMORY_USAGE_CPU_ONLY; }
		if (flags & Graphics::GPUBufferFlags::INDEX && flags & Graphics::GPUBufferFlags::TRANSFER_SRC) { usage = VMA_MEMORY_USAGE_CPU_ONLY; }
		if (flags & Graphics::GPUBufferFlags::VERTEX && flags & Graphics::GPUBufferFlags::TRANSFER_DST) { usage = VMA_MEMORY_USAGE_GPU_ONLY; }
		if (flags & Graphics::GPUBufferFlags::INDEX && flags & Graphics::GPUBufferFlags::TRANSFER_DST) { usage = VMA_MEMORY_USAGE_GPU_ONLY; }

		ASSERT(usage != VMA_MEMORY_USAGE_UNKNOWN && "[ToVMAMemoryUsage] GPUBufferFlag not setup.");

		return usage;
	}

	GPUBufferVulkan::GPUBufferVulkan()
		: m_buffer(nullptr)
	{ }

	GPUBufferVulkan::~GPUBufferVulkan()
	{
		ReleaseGPU();
	}

	void GPUBufferVulkan::Init(Graphics::GPUBufferDesc const& desc)
	{
		ASSERT(desc.Size > 0 && desc.Stride > 0);

		ReleaseGPU();
		m_desc = desc;

		VkBufferCreateInfo info = vks::initializers::bufferCreateInfo(ToVulkanBufferUsageFlags(m_desc.Flags), m_desc.Size);

		VmaAllocationCreateInfo vmaInfo = { };
		vmaInfo.usage = ToVMAMemoryUsage(m_desc.Flags);
		vmaInfo.flags = 0;
		vmaInfo.pool = VK_NULL_HANDLE;

		ThrowIfFailed(vmaCreateBuffer(m_device->VmaAllocator, &info, &vmaInfo, &m_buffer, &m_vmaAllocation, &m_vmaAllocationInfo));
		m_memoryUsage = m_desc.Size;

		if (m_desc.InitData != nullptr)
		{
			// Set the data. This can be done via set data call or via a staging buffer.
			if (m_desc.Flags & Graphics::GPUBufferFlags::TRANSFER_SRC)
			{
				// Memory should be mapable.
				SetData(m_desc.InitData, m_desc.Size);
			}
			else
			{
				// Using staging buffer.
			}
		}
	}

	void* GPUBufferVulkan::Map(GPUResourceMapMode mapMode)
	{
		ThrowIfFailed(vmaMapMemory(m_device->VmaAllocator, m_vmaAllocation, &m_mappedData));
		return m_mappedData;
	}

	void GPUBufferVulkan::UnMap()
	{
		vmaUnmapMemory(m_device->VmaAllocator, m_vmaAllocation);
		m_mappedData = nullptr;
	}

	void GPUBufferVulkan::Upload()
	{
		// Create staging buffer. 
		// Copy and upload to GPU. 
		// Check if the buffer is dynamic or not. 
		GPUBuffer* stagingBuffer = GPUBuffer::New();
		Graphics::GPUBufferDesc stagingBuffDesc = m_desc;
		stagingBuffDesc.Flags = Graphics::GPUBufferFlags::TRANSFER_SRC;
		stagingBuffer->Init(stagingBuffDesc);

		//Create command buffer
		// Copy command
		// submit buffer 
	}

	void GPUBufferVulkan::Download()
	{

	}

	void GPUBufferVulkan::OnReleaseGPU()
	{
		vmaDestroyBuffer(m_device->VmaAllocator, m_buffer, m_vmaAllocation);
		m_memoryUsage = 0;
	}
}

