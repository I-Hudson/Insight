#pragma once

#include "VulkanHeaders.h"
#include "Engine/Graphics/GPUDevice.h"
#include "Engine/Graphics/GPUResource.h"
#include "VulkanDebug.h"
#include "Config.h"
#include "Types.h"

class GPUContextVulkan;
class GPUDeviceVulkan;
class GPUAdapterVulkan;
class CmdBufferVulkan;
class QueueVulkan;
class FenceVulkan;
class RenderPassVulkan;
class FenceManagerVulkan;

class GPUTextureVulkan;
class GPUBufferVulkan;

class SemaphoreVulkan
{
public:

    /// <summary>
    /// Initializes a new instance of the <see cref="SemaphoreVulkan"/> class.
    /// </summary>
    /// <param name="device">The graphics device.</param>
    SemaphoreVulkan(GPUDeviceVulkan* device);

    /// <summary>
    /// Finalizes an instance of the <see cref="SemaphoreVulkan"/> class.
    /// </summary>
    ~SemaphoreVulkan();

    /// <summary>
    /// Gets the handle.
    /// </summary>
    /// <returns>The semaphore.</returns>
    INLINE VkSemaphore GetHandle() const
    {
        return _semaphoreHandle;
    }

private:
    GPUDeviceVulkan* _device;
    VkSemaphore _semaphoreHandle;
};

class FenceVulkan
{
public:
    FenceVulkan() = delete;
    FenceVulkan(GPUDeviceVulkan* device, FenceManagerVulkan* owner, bool createSignaled);
    ~FenceVulkan();

    INLINE VkFence GetHandle() const
    {
        return m_handle;
    }

    INLINE bool IsSignaled() const
    {
        return m_signaled;
    }

    FenceManagerVulkan* GetOwner() const
    {
        return m_owner;
    }

private:
    VkFence m_handle;
    bool m_signaled;
    FenceManagerVulkan* m_owner;

    friend  FenceManagerVulkan;
};

class FenceManagerVulkan
{
public:

    FenceManagerVulkan()
        : m_device(nullptr)
    {
    }

    ~FenceManagerVulkan();

    /// <summary>
    /// Initializes the specified device.
    /// </summary>
    /// <param name="device">The graphics device.</param>
    void Init(GPUDeviceVulkan* device)
    {
        m_device = device;
    }

    void Dispose();

    FenceVulkan* AllocateFence(bool createSignaled = false);

    inline bool IsFenceSignaled(FenceVulkan* fence)
    {
        if (fence->IsSignaled())
        {
            return true;
        }

        return CheckFenceState(fence);
    }

    // Returns true if waiting timed out or failed, false otherwise.
    bool WaitForFence(FenceVulkan* fence, U64 timeInNanoseconds);

    void ResetFence(FenceVulkan* fence);

    // Sets the fence handle to null
    void ReleaseFence(FenceVulkan*& fence);

    // Sets the fence handle to null
    void WaitAndReleaseFence(FenceVulkan*& fence, U64 timeInNanoseconds);

private:
    // Returns true if fence was signaled, otherwise false.
    bool CheckFenceState(FenceVulkan* fence);

    void DestroyFence(FenceVulkan* fence);

private:
    GPUDeviceVulkan* m_device;
    std::vector<FenceVulkan*> m_freeFences;
    std::vector<FenceVulkan*> m_usedFences;

};

class GPUDeviceVulkan : public GPUDevice
{
public:
    static GPUDevice* New();

	GPUDeviceVulkan();
	virtual ~GPUDeviceVulkan() override;

    virtual GPUContext* GetMainContext() override;
    virtual GPUAdapter* GetAdapter() override;
    virtual bool Init();
    virtual bool LoadContent();
    virtual bool CanDraw() { return true; }
    virtual void Dispose();
    virtual void WaitForGPU() override;

    virtual GPUBuffer* NewBuffer(const std::string& name = "") override;
    virtual GPUTexture* NewTexture(const std::string& name = "") override;

    struct OptionalVulkanDeviceExtensions
    {
        U32 HasKHRMaintenance1 : 1;
        U32 HasKHRMaintenance2 : 1;
        U32 HasMirrorClampToEdge : 1;
        U32 HasKHRExternalMemoryCapabilities : 1;
        U32 HasKHRGetPhysicalDeviceProperties2 : 1;
        U32 HasEXTValidationCache : 1;
    };
    static OptionalVulkanDeviceExtensions OptionalDeviceExtensions;

private:
    static void GetInstanceExtensions(std::vector<const char*>& instanceExtensions, std::vector<const char*>& layerExtensions);
    void GetDeviceExtensionsAndLayers(VkPhysicalDevice gpu, std::vector<const char*>& deviceExtensions, std::vector<const char*>& layerExtensions);

public:
    VkDevice Device;

    GPUContextVulkan* MainContext = nullptr;

    /// <summary>
    /// The Vulkan fence manager.
    /// </summary>
    FenceManagerVulkan FenceManager;

    /// <summary>
    /// Graphics queue.
    /// </summary>
    QueueVulkan* GraphicsQueue;

    /// <summary>
    /// Compute queue.
    /// </summary>
    QueueVulkan* ComputeQueue;

    /// <summary>
    /// Transfer queue.
    /// </summary>
    QueueVulkan* TransferQueue;

    /// <summary>
    /// The physical device limits.
    /// </summary>
    VkPhysicalDeviceLimits PhysicalDeviceLimits;

    /// <summary>
    /// The Vulkan memory allocator.
    /// </summary>
    VmaAllocator VmaAllocator = VK_NULL_HANDLE;

private:
    VkInstance m_instance;

    std::vector<VkQueueFamilyProperties> m_queueFamilyProps;

    VkPhysicalDeviceFeatures m_physicalDeviceFeatures;

    GPUAdapterVulkan* m_adapter = nullptr;

    CriticalSection m_fenceLock;

    friend GPUContextVulkan;
    friend FenceManagerVulkan;
};