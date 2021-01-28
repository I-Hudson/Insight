#pragma once

#include "Engine/Graphics/GPUResourceState.h"
#include "GPUDeviceVulkan.h"
#include "ResourceOwnerVulkan.h"

/// <summary>
/// The shared ring buffer for uniform buffers uploading for Vulkan backend.
/// </summary>
/// <seealso cref="GPUResourceVulkan{GPUResource}" />
/// <seealso cref="ResourceOwnerVulkan" />
class UniformBufferUploaderVulkan : public GPUResourceVulkan<GPUResource>, public ResourceOwnerVulkan
{
public:

    struct Allocation
    {
        /// <summary>
        /// The allocation offset from the GPU buffer begin (in bytes).
        /// </summary>
        U64 Offset;

        /// <summary>
        /// The allocation size (in bytes).
        /// </summary>
        U64 Size;

        /// <summary>
        /// The GPU buffer.
        /// </summary>
        VkBuffer Buffer;

        /// <summary>
        /// The CPU memory address to the mapped buffer data. Can be used to write the uniform buffer contents to upload them to GPU.
        /// </summary>
        Byte* CPUAddress;
    };


    /// <summary>
    /// Initializes a new instance of the <see cref="UniformBufferUploaderVulkan"/> class.
    /// </summary>
    /// <param name="device">The graphics device.</param>
    UniformBufferUploaderVulkan(GPUDeviceVulkan* device);

    Allocation Allocate(U64 size, U32 alignment, CmdBufferVulkan* cmdBuffer);

    // [GPUResourceVulkan]
    virtual ResourceType GetResourceType() const final override
    {
        return ResourceType::Buffer;
    }

    // [ResourceOwnerVulkan]
    virtual GPUResource* AsGPUResource() const override
    {
        return (GPUResource*)this;
    }

protected:

    // [GPUResourceVulkan]
    virtual void OnReleaseGPU() override;

private:

    VkBuffer m_buffer;
    VmaAllocation m_allocation;
    U64 m_size;
    U64 m_offset;
    U32 m_minAlignment;
    Byte* m_mapped;
    CmdBufferVulkan* m_fenceCmdBuffer;
    U64 m_fenceCounter;
};

