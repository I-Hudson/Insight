#pragma once

#include "GPUDeviceVulkan.h"
#include "Engine/Core/InsightAlias.h"
#include "GPUTimerQueryVulkan.h"

class GPUDeviceVulkan;
class CmdBufferPoolVulkan;
class QueueVulkan;
class DescriptorPoolSetContainerVulkan;

class CmdBufferVulkan
{
public:
	enum class State
	{
		ReadyForBegin,
		IsInsideBegin,
		IsInsideRenderPass,
		HasEnded,
		Submitted,
	};

    CmdBufferVulkan(GPUDeviceVulkan* device, CmdBufferPoolVulkan* pool);

    ~CmdBufferVulkan();

    CmdBufferPoolVulkan* GetOwner() const
    {
        return m_commandBufferPool;
    }

    State GetState() const
    {
        return m_state;
    }

    FenceVulkan* GetFence() const
    {
        return m_fence;
    }

    INLINE bool IsInsideRenderPass() const
    {
        return m_state == State::IsInsideRenderPass;
    }

    INLINE bool IsOutsideRenderPass() const
    {
        return m_state == State::IsInsideBegin;
    }

    INLINE bool HasBegun() const
    {
        return m_state == State::IsInsideBegin || m_state == State::IsInsideRenderPass;
    }

    INLINE bool HasEnded() const
    {
        return m_state == State::HasEnded;
    }

    INLINE bool IsSubmitted() const
    {
        return m_state == State::Submitted;
    }

    INLINE VkCommandBuffer GetHandle() const
    {
        return m_commandBuffer;
    }

    INLINE volatile U64 GetFenceSignaledCounter() const
    {
        return m_fenceSignaledCounter;
    }

    INLINE volatile U64 GetSubmittedFenceCounter() const
    {
        return m_submittedFenceCounter;
    }

    void AddWaitSemaphore(VkPipelineStageFlags waitFlags, SemaphoreVulkan* waitSemaphore);

    void Begin();
    void End();

    void BeginRenderPass(RenderPassVulkan* renderPass, FramebufferVulkan* framebuffer, U32 clearValueCount, VkClearValue* clearValues);
    void EndRenderPass();

    DescriptorPoolSetContainerVulkan* GetDescriptorPoolSet() const
    {
        return m_descriptorPoolSetContainer;
    }

    void AcquirePoolSet();

#if GPU_ALLOW_PROFILE_EVENTS
    void BeginEvent(const char* name);
    void EndEvent();
#endif

    void RefreshFenceStatus();

private:
    GPUDeviceVulkan* m_device;
    VkCommandBuffer m_commandBuffer;
    State m_state;

    std::vector<VkPipelineStageFlags> m_waitFlags;
    std::vector<SemaphoreVulkan*> m_waitSemaphores;
    std::vector<SemaphoreVulkan*> m_submittedWaitSemaphores;

    FenceVulkan* m_fence;
#if GPU_ALLOW_PROFILE_EVENTS
    I32 m_eventsBegin = 0;
#endif

    // The latest value when command buffer was submitted.
    volatile U64 m_submittedFenceCounter;

    // The latest value passed after the fence was signaled.
    volatile U64 m_fenceSignaledCounter;

    CmdBufferPoolVulkan* m_commandBufferPool;

    DescriptorPoolSetContainerVulkan* m_descriptorPoolSetContainer = nullptr;

	friend QueueVulkan;
};

class CmdBufferPoolVulkan
{
public:
    CmdBufferPoolVulkan(GPUDeviceVulkan* device);
    ~CmdBufferPoolVulkan();

    INLINE VkCommandPool GetHandle() const
    {
        ASSERT(m_handle != VK_NULL_HANDLE);
        return m_handle;
    }

    void RefreshFenceStatus(CmdBufferVulkan* skipCmdBuffer = nullptr);

private:
    GPUDeviceVulkan* m_device;
    VkCommandPool m_handle;
    std::vector<CmdBufferVulkan*> m_cmdBuffers;

    CmdBufferVulkan* Create();

    void Create(U32 queueFamilyIndex);

    friend class CmdBufferManagerVulkan;
};

class CmdBufferManagerVulkan
{
    CmdBufferManagerVulkan(GPUDeviceVulkan* device, GPUContextVulkan* context);

    void SubmitActiveCmdBuffer(SemaphoreVulkan* signalSemaphore = nullptr);

    void WaitForCmdBuffer(CmdBufferVulkan* cmdBuffer, float timeInSecondsToWait = 1.0f);

    void RefreshFenceStatus(CmdBufferVulkan* skipCmdBuffer = nullptr)
    {
        m_pool.RefreshFenceStatus(skipCmdBuffer);
    }

    void PrepareForNewActiveCommandBuffer();

    void OnQueryBegin(GPUTimerQueryVulkan* query);
    void OnQueryEnd(GPUTimerQueryVulkan* query);

    FORCE_INLINE VkCommandPool GetHandle() const
    {
        return m_pool.GetHandle();
    }

    FORCE_INLINE CmdBufferVulkan* GetActiveCmdBuffer() const
    {
        return m_activeCmdBuffer;
    }

    FORCE_INLINE bool HasPendingActiveCmdBuffer() const
    {
        return m_activeCmdBuffer != nullptr;
    }

    FORCE_INLINE bool HasQueriesInProgress() const
    {
        return m_queriesInProgress.size() != 0;
    }

    CmdBufferVulkan* GetCmdBuffer()
    {
        if (!m_activeCmdBuffer)
            PrepareForNewActiveCommandBuffer();
        return m_activeCmdBuffer;
    }

private:
    GPUDeviceVulkan* m_device;
    CmdBufferPoolVulkan m_pool;
    QueueVulkan* m_queue;
    CmdBufferVulkan* m_activeCmdBuffer;
    std::vector<GPUTimerQueryVulkan*> m_queriesInProgress;
};