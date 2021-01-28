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
class UniformBufferUploaderVulkan;
class DescriptorPoolsManagerVulkan;

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

/// <summary>
/// Deferred Deletion Queue Vulkan
/// </summary>
class DeferredDeletionQueueVulkan
{
public:
    enum Type
    {
        RenderPass,
        Buffer,
        BufferView,
        Image,
        ImageView,
        Pipeline,
        PipelineLayout,
        Framebuffer,
        DescriptorSetLayout,
        Sampler,
        Semaphore,
        ShaderModule,
        Event,
        QueryPool,
    };

    /// <summary>
    /// Initializes a new instance of the <see cref="DeferredDeletionQueueVulkan"/> class.
    /// </summary>
    /// <param name="device">The graphics device.</param>
    DeferredDeletionQueueVulkan(GPUDeviceVulkan* device);

    /// <summary>
    /// Finalizes an instance of the <see cref="DeferredDeletionQueueVulkan"/> class.
    /// </summary>
    ~DeferredDeletionQueueVulkan();

    template<typename T>
    inline void EnqueueResource(Type type, T handle)
    {
        static_assert(sizeof(T) <= sizeof(U64), "Invalid handle size.");
        EnqueueGenericResource(type, (U64)handle, VK_NULL_HANDLE);
    }

    template<typename T>
    inline void EnqueueResource(Type type, T handle, VmaAllocation allocation)
    {
        static_assert(sizeof(T) <= sizeof(U64), "Invalid handle size.");
        EnqueueGenericResource(type, (U64)handle, allocation);
    }

    auto ReleaseResources(bool deleteImmediately = false) -> void;

private:
    void EnqueueGenericResource(Type type, U64 handle, VmaAllocation allocation);

private:
    struct Entry
    {
        U64 FenceCounter;
        U64 Handle;
        U64 FrameNumber;
        VmaAllocation AllocationHandle;
        Type StructureType;
        CmdBufferVulkan* CmdBuffer;
    };

    GPUDeviceVulkan* _device;
    CriticalSection _locker;
    std::vector<Entry> _entries;

};

/// <summary>
/// Render target layout
/// </summary>
class RenderTargetLayoutVulkan
{
public:
    bool operator==(const RenderTargetLayoutVulkan& other) const
    {
        return Platform::MemCompare((void*)this, &other, sizeof(RenderTargetLayoutVulkan)) == 0;
    }

public:
    I32 RTsCount;
    MSAALevel MSAA;
    bool ReadDepth;
    bool WriteDepth;
    PixelFormat DepthFormat;
    PixelFormat RTVsFormats[GPU_MAX_RT_BINDED];
    VkExtent3D Extent;
};
U32 GetHash(const RenderTargetLayoutVulkan& key);

/// <summary>
/// Framebuffer
/// </summary>
class FramebufferVulkan
{
public:
    struct Key
    {
        RenderPassVulkan* RenderPass;
        I32 AttachmentCount;
        VkImageView Attachments[GPU_MAX_RT_BINDED + 1];

    public:

        bool operator==(const Key& other) const
        {
            return Platform::MemCompare((void*)this, &other, sizeof(Key)) == 0;
        }
    };

    FramebufferVulkan(GPUDeviceVulkan* device, Key& key, VkExtent3D& extent, U32 layers);
    ~FramebufferVulkan();


    inline VkFramebuffer GetHandle()
    {
        return _handle;
    }

    bool HasReference(VkImageView imageView) const;

public:
    VkImageView Attachments[GPU_MAX_RT_BINDED + 1];
    VkExtent3D Extent;
    U32 Layers;

private:

    GPUDeviceVulkan* _device;
    VkFramebuffer _handle;
};
U32 GetHash(const FramebufferVulkan::Key& key);

/// <summary>
/// Render pass
/// </summary>
class RenderPassVulkan
{
public:
    RenderPassVulkan(GPUDeviceVulkan* device, const RenderTargetLayoutVulkan& layout);
    ~RenderPassVulkan();

    INLINE VkRenderPass GetHandle() const { return m_handle; }

public:
    RenderTargetLayoutVulkan Layout;

private:
    GPUDeviceVulkan* m_device;
    VkRenderPass m_handle;
};

/// <summary>
/// Query pool
/// </summary>
class QueryPoolVulkan
{
public:
    QueryPoolVulkan(GPUDeviceVulkan* device, I32 capacity, VkQueryType type);
    ~QueryPoolVulkan();

    INLINE VkQueryPool GetHandle() const
    {
        return _handle;
    }

#if VULKAN_RESET_QUERY_POOLS
    void Reset(CmdBufferVulkan* cmdBuffer);
#endif

protected:
    struct Range
    {
        U32 Start;
        U32 Count;
    };

    GPUDeviceVulkan* _device;
    VkQueryPool _handle;

    volatile I32 _count;
    const U32 _capacity;
    const VkQueryType _type;
#if VULKAN_RESET_QUERY_POOLS
    Array<Range> _resetRanges;
#endif

};

class BufferedQueryPoolVulkan : public QueryPoolVulkan
{
public:
    BufferedQueryPoolVulkan(GPUDeviceVulkan* device, I32 capacity, VkQueryType type);
    bool AcquireQuery(U32& resultIndex);
    void ReleaseQuery(U32 queryIndex);
    void MarkQueryAsStarted(U32 queryIndex);
    bool GetResults(GPUContextVulkan* context, U32 index, U64& result);
    bool HasRoom() const;

private:
   std::vector<U64> _queryOutput;
   std::vector<U64> _usedQueryBits;
   std::vector<U64> _startedQueryBits;
   std::vector<U64> _readResultsBits;

    // Last potentially free index in the pool
    I32 _lastBeginIndex;
};

/// <summary>
/// The dummy Vulkan resources manager. Helps when user need to pass null texture handle to the shader.
/// </summary>
class HelperResourcesVulkan
{
public:

    enum class StaticSamplers
    {
        SamplerLinearClamp = 0,
        SamplerPointClamp = 1,
        SamplerLinearWrap = 2,
        SamplerPointWrap = 3,
        ShadowSampler = 4,
        ShadowSamplerPCF = 5,

        MAX
    };


    /// <summary>
    /// Initializes a new instance of the <see cref="DummyResourcesVulkan"/> class.
    /// </summary>
    /// <param name="device">The graphics device.</param>
    HelperResourcesVulkan(GPUDeviceVulkan* device);

    VkSampler GetStaticSampler(StaticSamplers type);

    GPUTextureVulkan* GetDummyTexture(SpirvShaderResourceType type);

    GPUBufferVulkan* GetDummyBuffer();

    GPUBufferVulkan* GetDummyVertexBuffer();

    void Dispose();

private:
    GPUDeviceVulkan* _device;
    GPUTextureVulkan* _dummyTextures[6];
    GPUBufferVulkan* _dummyBuffer;
    GPUBufferVulkan* _dummyVB;
    VkSampler _staticSamplers[static_cast<I32>(StaticSamplers::MAX)];
};

struct PendingItemsPerCmdBuffer;
/// <summary>
/// Vulkan staging buffers manager.
/// </summary>
class StagingManagerVulkan
{
public:

    StagingManagerVulkan(GPUDeviceVulkan* device);
    GPUBuffer* AcquireBuffer(U32 size, GPUResourceUsage usage);
    void ReleaseBuffer(CmdBufferVulkan* cmdBuffer, GPUBuffer*& buffer);
    void ProcessPendingFree();
    void Dispose();

private:
    struct PendingItems
    {
        U64 FenceCounter;
        std::vector<GPUBuffer*> Resources;
    };

    struct PendingItemsPerCmdBuffer
    {
        CmdBufferVulkan* CmdBuffer;
        std::vector<PendingItems> Items;

        INLINE PendingItems* FindOrAdd(U64 fence);
    };

    PendingItemsPerCmdBuffer* FindOrAdd(CmdBufferVulkan* cmdBuffer);

    struct FreeEntry
    {
        GPUBuffer* Buffer;
        U64 FrameNumber;
    };

    GPUDeviceVulkan* _device;
    CriticalSection _locker;
    std::vector<GPUBuffer*> _allBuffers;
    std::vector<FreeEntry> _freeBuffers;
    std::vector<PendingItemsPerCmdBuffer> _pendingBuffers;
#if !IS_RELEASE
    U64 _allBuffersTotalSize = 0;
    U64 _allBuffersPeekSize = 0;
    U64 _allBuffersAllocSize = 0;
    U64 _allBuffersFreeSize = 0;
#endif
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
    /// The Vulkan resources deferred deletion queue.
    /// </summary>
    DeferredDeletionQueueVulkan DeferredDeletionQueue;

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

    /// <summary>
    /// The uniform buffer uploader.
    /// </summary>
    UniformBufferUploaderVulkan* UniformBufferUploader = nullptr;

    /// <summary>
    /// The descriptor pools manager.
    /// </summary>
    DescriptorPoolsManagerVulkan* DescriptorPoolsManager = nullptr;

private:
    VkInstance m_instance;

    std::vector<VkQueueFamilyProperties> m_queueFamilyProps;

    VkPhysicalDeviceFeatures m_physicalDeviceFeatures;

    GPUAdapterVulkan* m_adapter = nullptr;

    CriticalSection m_fenceLock;

    friend GPUContextVulkan;
    friend FenceManagerVulkan;
};

/// <summary>
/// GPU resource implementation for Vulkan backend.
/// </summary>
template<class BaseType>
class GPUResourceVulkan : public GPUResourceBase<GPUDeviceVulkan, BaseType>
{
public:

    /// <summary>
    /// Initializes a new instance of the <see cref="GPUResourceVulkan"/> class.
    /// </summary>
    /// <param name="device">The graphics device.</param>
    /// <param name="name">The resource name.</param>
    GPUResourceVulkan(GPUDeviceVulkan* device, const std::string& name)
        : GPUResourceBase<GPUDeviceVulkan, BaseType>(device, name)
    {
    }
};