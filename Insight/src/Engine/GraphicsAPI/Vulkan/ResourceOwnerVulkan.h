#pragma once

#include "Engine/Graphics/GPUResourceState.h"
#include "VulkanHeaders.h"

class GPUResource;
class GPUContextVulkan;

/// <summary>
/// Vulkan resource state used to indicate invalid state (useful for debugging resource tracking issues).
/// </summary>
#define VK_IMAGE_LAYOUT_CORRUPT VK_IMAGE_LAYOUT_MAX_ENUM

/// <summary>
/// Tracking of per-resource or per-subresource state for Vulkan resources that require to issue resource access barriers during rendering.
/// </summary>
class ResourceStateVulkan : public GPUResourceState<VkImageLayout, VK_IMAGE_LAYOUT_CORRUPT>
{
};

/// <summary>
/// Base class for objects in Vulkan backend that can own a resource.
/// </summary>
class ResourceOwnerVulkan
{

public:

    virtual ~ResourceOwnerVulkan()
    {
    }

    /// <summary>
    /// Gets resource owner object as a GPUResource type or returns null if cannot perform cast.
    /// </summary>
    /// <returns>GPU Resource or null if cannot cast.</returns>
    virtual GPUResource* AsGPUResource() const = 0;

public:

    /// <summary>
    /// The resource state tracking helper. Used for resource barriers.
    /// </summary>
    ResourceStateVulkan State;

    /// <summary>
    /// The array size (for textures).
    /// </summary>
    I32 ArraySlices;

protected:

    void InitResource(VkImageLayout initialState, I32 mipLevels = 1, I32 arraySize = 1, bool usePerSubresourceTracking = false)
    {
        State.Initialize(mipLevels * arraySize, initialState, usePerSubresourceTracking);
        ArraySlices = arraySize;
    }

private:
    friend GPUContextVulkan;
};