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
class PipelineEventManagerVulkan;

class GPUTextureVulkan;
class GPUBufferVulkan;

class PipelineEventVulkan
{
public:
	PipelineEventVulkan(GPUDeviceVulkan* device, PipelineEventManagerVulkan* owner);

	~PipelineEventVulkan() { ASSERT(m_event == VK_NULL_HANDLE && "[~PipelineEventVulkan] Not deleted."); }

	INLINE VkEvent GetHandle() const { return m_event; }

	INLINE VkPipelineStageFlags GetStages() const { return m_stages; }

	INLINE void SetStages(const VkPipelineStageFlags& stages) { m_stages = stages; }

	INLINE bool IsSignaled() const;

	PipelineEventManagerVulkan* GetOwner() const { return m_owner; }

private:
	PipelineEventManagerVulkan* m_owner;
	bool m_signaled;
	VkEvent m_event;
	VkPipelineStageFlags m_stages = 0;

	friend class PipelineEventManagerVulkan;
};

class PipelineEventManagerVulkan
{
public:
	PipelineEventManagerVulkan();

	~PipelineEventManagerVulkan();

	void Init(GPUDeviceVulkan* device) { m_device = device; }

	void Dispose();

	PipelineEventVulkan* AllocateEvent(bool createSignaled);

	bool IsEventSignaled(PipelineEventVulkan* event);

	void ResetEvent(PipelineEventVulkan*& event);

	void ReleaseEvent(PipelineEventVulkan*& events);

private:
	bool CheckForEventStatus(PipelineEventVulkan* event);

	void DestroyEvent(PipelineEventVulkan* event);

private:
	GPUDeviceVulkan* m_device;

	std::vector<PipelineEventVulkan*> m_freeEvents;
	std::vector<PipelineEventVulkan*> m_usedEvents;
};

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
	void Init(GPUDeviceVulkan* device) { m_device = device; }

	void Dispose();

	FenceVulkan* AllocateFence(bool createSignaled = false);

	INLINE bool IsFenceSignaled(FenceVulkan* fence)
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

	VkQueue GetQueue(GPUQueue queue);
	u32 GetQueueFamilyIndex(GPUQueue queue);

	virtual GPUContext* GetMainContext() override;
	virtual GPUAdapter* GetAdapter() override;
	virtual bool Init();
	virtual bool LoadContent();
	virtual bool CanDraw() { return true; }
	virtual void Dispose();
	virtual void WaitForGPU() override;
	virtual u32 GetQueueIndex(GPUQueue queue) override;

	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual GPUImageView* GetTransientAttachment(U32 width, U32 height, PixelFormat format, U32 index, U32 samples, U32 layers) override;

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

	PipelineEventManagerVulkan PipelineEventManger;

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

	std::unordered_map<U32, std::pair<bool, GPUImage*>> m_trasientImages;

	CriticalSection m_fenceLock;
	CriticalSection m_pipelineEventLock;

	//TODO: Remove this when QueueVulkan is made.
	u32 m_graphicsQueueIndex;
	u32 m_computeQueueIndex;
	u32 m_transferQueueIndex;
	u32 m_graphicsQueueFamilyIndex;
	u32 m_computeQueueFamilyIndex;
	u32 m_transferQueueFamilyIndex;
	VkQueue m_graphicsQueue;
	VkQueue m_computeQueue;
	VkQueue m_transferQueue;

	friend GPUContextVulkan;
	friend FenceManagerVulkan;
	friend PipelineEventManagerVulkan;
};

template<typename Resource>
class GPUResouceVulkan : public GPUResouceBase<GPUDeviceVulkan, Resource>
{
public:
};