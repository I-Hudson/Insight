#include "ispch.h"
#include "UniformBufferUploaderVulkan.h"
#include "GPUDeviceVulkan.h"
#include "VulkanInitializers.h"
#include "VulkanUtils.h"

#if PLATFORM_DESKTOP
#define VULKAN_UNIFORM_RING_BUFFER_SIZE 16 * 1024 * 1024
#else
#define VULKAN_UNIFORM_RING_BUFFER_SIZE 8 * 1024 * 1024
#endif

UniformBufferUploaderVulkan::UniformBufferUploaderVulkan(GPUDeviceVulkan* device)
    : GPUResourceVulkan(device, "Uniform Buffer Uploader")
    , m_size(VULKAN_UNIFORM_RING_BUFFER_SIZE)
    , m_offset(0)
    , m_mapped(nullptr)
    , m_fenceCmdBuffer(nullptr)
    , m_fenceCounter(0)
{
    m_minAlignment = (U32)device->PhysicalDeviceLimits.minUniformBufferOffsetAlignment;

    // Setup buffer description
    VkBufferCreateInfo bufferInfo = vks::initializers::bufferCreateInfo();
    bufferInfo.size = m_size;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    // Create buffer
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    VkResult result = vmaCreateBuffer(m_device->VmaAllocator, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
    ThrowIfFailed(result);
    m_memoryUsage = bufferInfo.size;

    // Map buffer
    result = vmaMapMemory(m_device->VmaAllocator, m_allocation, (void**)&m_mapped);
    ThrowIfFailed(result);
}

UniformBufferUploaderVulkan::Allocation UniformBufferUploaderVulkan::Allocate(U64 size, U32 alignment, CmdBufferVulkan* cmdBuffer)
{
    alignment = std::max(m_minAlignment, alignment);
    U64 offset = AlignUp<U64>(m_offset, alignment);

    // Check if wrap around ring buffer
    if (offset + size >= m_size)
    {
        if (m_fenceCmdBuffer)
        {
            ASSERT(false && "Uncomment code.");
            //if (m_fenceCmdBuffer == cmdBuffer && m_fenceCounter == cmdBuffer->GetFenceSignaledCounter())
            {
                IS_ERROR("Wrapped around the ring buffer. Requested more bytes than possible in the same cmd buffer!");
            }
            //else if (m_fenceCounter == m_fenceCmdBuffer->GetFenceSignaledCounter())
            {
                IS_ERROR("Wrapped around the ring buffer! Need to wait on the GPU!!!");
            }
        }

        offset = 0;
        m_offset = size;

        m_fenceCmdBuffer = cmdBuffer;
        //m_fenceCounter = cmdBuffer->GetSubmittedFenceCounter();
    }
    else
    {
        // Move within the buffer
        m_offset = offset + size;
    }

    Allocation result;
    result.Offset = offset;
    result.Size = size;
    result.Buffer = m_buffer;
    result.CPUAddress = m_mapped + offset;
    return result;
}

void UniformBufferUploaderVulkan::OnReleaseGPU()
{
    if (m_allocation != VK_NULL_HANDLE)
    {
        if (m_mapped)
        {
            vmaUnmapMemory(m_device->VmaAllocator, m_allocation);
            m_mapped = nullptr;
        }
        vmaDestroyBuffer(m_device->VmaAllocator, m_buffer, m_allocation);
        m_buffer = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
    }
}