#include "ispch.h"
#include "QueueVulkan.h"
#include "GPUDeviceVulkan.h"
#include "VulkanInitializers.h"
#include "CmdBufferVulkan.h"
#include "VulkanUtils.h"

QueueVulkan::QueueVulkan(GPUDeviceVulkan* device, U32 familyIndex)
	: m_queue(VK_NULL_HANDLE)
	, m_familyIndex(familyIndex)
	, m_queueIndex(0)
	, m_device(device)
	, m_lastSubmittedCmdBuffer(nullptr)
	, m_lastSubmittedCmdBufferFenceCounter(0)
	, m_submitCounter(0)
{
	vkGetDeviceQueue(device->Device, m_familyIndex, 0, &m_queue);
}

void QueueVulkan::Submit(CmdBufferVulkan* cmdBuffer, U32 numSignalSemaphores, VkSemaphore* signalSemaphores)
{
    ASSERT(false && "THIS MUST BE IMPLERMENTED!!!!");

    ASSERT(cmdBuffer->HasEnded());

    auto fence = cmdBuffer->GetFence();
    ASSERT(!fence->IsSignaled());

    const VkCommandBuffer cmdBuffers[] = { cmdBuffer->GetHandle() };

    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = cmdBuffers;
    submitInfo.signalSemaphoreCount = numSignalSemaphores;
    submitInfo.pSignalSemaphores = signalSemaphores;

    std::vector<VkSemaphore> waitSemaphores;
    if (!cmdBuffer->m_waitSemaphores.empty())
    {
        waitSemaphores.reserve((U32)cmdBuffer->m_waitSemaphores.size());
        for (auto semaphore : cmdBuffer->m_waitSemaphores)
        {
            waitSemaphores.push_back(semaphore->GetHandle());
        }
        submitInfo.waitSemaphoreCount = (U32)cmdBuffer->m_waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = cmdBuffer->m_waitFlags.data();
    }

    ThrowIfFailed(vkQueueSubmit(m_queue, 1, &submitInfo, fence->GetHandle()));

    // Mark semaphores as submitted
    cmdBuffer->m_state = CmdBufferVulkan::State::Submitted;
    cmdBuffer->m_waitFlags.clear();
    cmdBuffer->m_submittedWaitSemaphores = cmdBuffer->m_waitSemaphores;
    cmdBuffer->m_waitSemaphores.clear();
    cmdBuffer->m_submittedFenceCounter = cmdBuffer->m_fenceSignaledCounter;

#if 0
    // Wait for the GPU to be idle on every submit (useful for tracking GPU hangs)
    const bool WaitForIdleOnSubmit = false;
    if (WaitForIdleOnSubmit)
    {
        // Use 200ms timeout
        bool success = _device->FenceManager.WaitForFence(fence, 200 * 1000 * 1000);
        ASSERT(success);
        ASSERT(_device->FenceManager.IsFenceSignaled(fence));
        cmdBuffer->GetOwner()->RefreshFenceStatus();
    }
#endif

    UpdateLastSubmittedCommandBuffer(cmdBuffer);

    cmdBuffer->GetOwner()->RefreshFenceStatus(cmdBuffer); ASSERT(cmdBuffer->HasEnded());

    auto fence = cmdBuffer->GetFence();
    ASSERT(!fence->IsSignaled());

    const VkCommandBuffer cmdBuffers[] = { cmdBuffer->GetHandle() };

    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = cmdBuffers;
    submitInfo.signalSemaphoreCount = numSignalSemaphores;
    submitInfo.pSignalSemaphores = signalSemaphores;

    std::vector<VkSemaphore> waitSemaphores;
    if (!cmdBuffer->m_waitSemaphores.empty())
    {
        waitSemaphores.reserve((U32)cmdBuffer->m_waitSemaphores.size());
        for (auto semaphore : cmdBuffer->m_waitSemaphores)
        {
            waitSemaphores.push_back(semaphore->GetHandle());
        }
        submitInfo.waitSemaphoreCount = (U32)cmdBuffer->m_waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = cmdBuffer->m_waitFlags.data();
    }

    ThrowIfFailed(vkQueueSubmit(m_queue, 1, &submitInfo, fence->GetHandle()));

    // Mark semaphores as submitted
    cmdBuffer->m_state = CmdBufferVulkan::State::Submitted;
    cmdBuffer->m_waitFlags.clear();
    cmdBuffer->m_submittedWaitSemaphores = cmdBuffer->m_waitSemaphores;
    cmdBuffer->m_waitSemaphores.clear();
    cmdBuffer->m_submittedFenceCounter = cmdBuffer->m_fenceSignaledCounter;

#if 0
    // Wait for the GPU to be idle on every submit (useful for tracking GPU hangs)
    const bool WaitForIdleOnSubmit = false;
    if (WaitForIdleOnSubmit)
    {
        // Use 200ms timeout
        bool success = _device->FenceManager.WaitForFence(fence, 200 * 1000 * 1000);
        ASSERT(success);
        ASSERT(_device->FenceManager.IsFenceSignaled(fence));
        cmdBuffer->GetOwner()->RefreshFenceStatus();
    }
#endif

    UpdateLastSubmittedCommandBuffer(cmdBuffer);

    cmdBuffer->GetOwner()->RefreshFenceStatus(cmdBuffer);
}

void QueueVulkan::GetLastSubmittedInfo(CmdBufferVulkan*& cmdBuffer, U64& fenceCounter) const
{
	m_locker.Lock();

	cmdBuffer = m_lastSubmittedCmdBuffer;
	fenceCounter = m_lastSubmittedCmdBufferFenceCounter;

	m_locker.Unlock();
}

void QueueVulkan::UpdateLastSubmittedCommandBuffer(CmdBufferVulkan* cmdBuffer)
{
	m_locker.Lock();

	m_lastSubmittedCmdBuffer = cmdBuffer;
	//m_lastSubmittedCmdBufferFenceCounter = cmdBuffer->GetFenceSignaledCounter();
	m_submitCounter++;

	m_locker.Unlock();
}