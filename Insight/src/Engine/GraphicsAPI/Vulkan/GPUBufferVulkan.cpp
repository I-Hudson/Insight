#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"

namespace Insight::GraphicsAPI::Vulkan
{
	VmaMemoryUsage ToVMAMemoryUsage(Graphics::GPUBufferFlags const& flags, bool& mustBeMapped)
	{
		VmaMemoryUsage usage = VMA_MEMORY_USAGE_UNKNOWN;
		if (flags & Graphics::GPUBufferFlags::TRANSFER_SRC) { usage = VMA_MEMORY_USAGE_CPU_ONLY; mustBeMapped = true; }
		if (flags & Graphics::GPUBufferFlags::TRANSFER_DST) { usage = VMA_MEMORY_USAGE_GPU_TO_CPU; mustBeMapped = true; }

		if (flags & Graphics::GPUBufferFlags::UNIFORM) { usage = VMA_MEMORY_USAGE_CPU_TO_GPU; mustBeMapped = true; }

		if (flags & Graphics::GPUBufferFlags::VERTEX && flags & Graphics::GPUBufferFlags::TRANSFER_SRC) { usage = VMA_MEMORY_USAGE_CPU_ONLY;  mustBeMapped = true; }
		if (flags & Graphics::GPUBufferFlags::INDEX && flags & Graphics::GPUBufferFlags::TRANSFER_SRC) { usage = VMA_MEMORY_USAGE_CPU_ONLY;  mustBeMapped = true; }
		if (flags & Graphics::GPUBufferFlags::VERTEX && flags & Graphics::GPUBufferFlags::TRANSFER_DST) { usage = VMA_MEMORY_USAGE_GPU_TO_CPU; mustBeMapped = true;}
		if (flags & Graphics::GPUBufferFlags::INDEX && flags & Graphics::GPUBufferFlags::TRANSFER_DST) { usage = VMA_MEMORY_USAGE_GPU_TO_CPU; mustBeMapped = true;}

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
		m_vmaMemoryUsage = ToVMAMemoryUsage(m_desc.Flags, m_mustBeMapped);
		vmaInfo.usage = m_vmaMemoryUsage;

		ThrowIfFailed(vmaCreateBuffer(m_device->VmaAllocator, &info, &vmaInfo, &m_buffer, &m_vmaAllocation, &m_vmaAllocationInfo));
		m_memoryUsage = m_desc.Size;

		if (m_desc.InitData != nullptr)
		{
			SetData(m_desc.InitData, m_desc.Size);
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

	void GPUBufferVulkan::Upload(void* mapped, void const* data, u32 const& size)
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
			ASSERT(m_desc.Flags & Graphics::GPUBufferFlags::TRANSFER_SRC && "[GPUBufferVulkan::Download] GPUBuffer must have 'GPUBufferFlags::TRANSFER_SRC' set or use a different VMA memory usage.");

			// Create staging buffer. 
			GPUBufferVulkan stagingBuffer;
			Graphics::GPUBufferDesc stagingBuffDesc = { };
			stagingBuffDesc.Flags = Graphics::GPUBufferFlags::TRANSFER_DST;
			stagingBuffDesc.Size = m_desc.Size;
			stagingBuffDesc.Stride = m_desc.Size;
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

