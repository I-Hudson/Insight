#include "ispch.h"
#include "CmdBufferVulkan.h"
#include "QueueVulkan.h"
//#include "GPUContextVulkan.h"
//#include "GPUTimerQueryVulkan.h"
#include "DescriptorSetVulkan.h"
#include "VulkanUtils.h"
#include "VulkanInitializers.h"

void CmdBufferVulkan::AddWaitSemaphore(VkPipelineStageFlags waitFlags, SemaphoreVulkan* waitSemaphore)
{
    m_waitFlags.push_back(waitFlags);
    ASSERT(!VectorContains(m_waitSemaphores ,waitSemaphore));
    m_waitSemaphores.push_back(waitSemaphore);
}

void CmdBufferVulkan::Begin()
{
    ASSERT(m_state == State::ReadyForBegin);

    VkCommandBufferBeginInfo beginInfo = vks::initializers::commandBufferBeginInfo();
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    ThrowIfFailed(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));

    // Acquire a descriptor pool set on
    if (m_descriptorPoolSetContainer == nullptr)
    {
        AcquirePoolSet();
    }

    m_state = State::IsInsideBegin;

#if GPU_ALLOW_PROFILE_EVENTS
    // Reset events counter
    m_eventsBegin = 0;
#endif
}

void CmdBufferVulkan::End()
{
    ASSERT(IsOutsideRenderPass());

#if GPU_ALLOW_PROFILE_EVENTS
    // End remaining events
    while (m_eventsBegin--)
        vkCmdEndDebugUtilsLabelEXT(GetHandle());
#endif

    ThrowIfFailed(vkEndCommandBuffer(GetHandle()));
    m_state = State::HasEnded;
}

void CmdBufferVulkan::BeginRenderPass(RenderPassVulkan* renderPass, FramebufferVulkan* framebuffer, uint32 clearValueCount, VkClearValue* clearValues)
{
    ASSERT(IsOutsideRenderPass());

    VkRenderPassBeginInfo info = vks::initializers::renderPassBeginInfo();
    info.renderPass = renderPass->GetHandle();
    info.framebuffer = framebuffer->GetHandle();
    info.renderArea.offset.x = 0;
    info.renderArea.offset.y = 0;
    info.renderArea.extent.width = framebuffer->Extent.width;
    info.renderArea.extent.height = framebuffer->Extent.height;
    info.clearValueCount = clearValueCount;
    info.pClearValues = clearValues;

    vkCmdBeginRenderPass(m_commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    m_state = State::IsInsideRenderPass;
}

void CmdBufferVulkan::EndRenderPass()
{
    ASSERT(IsInsideRenderPass());
    vkCmdEndRenderPass(m_commandBuffer);
    m_state = State::IsInsideBegin;
}

void CmdBufferVulkan::AcquirePoolSet()
{
    ASSERT(!m_descriptorPoolSetContainer);
    m_descriptorPoolSetContainer = &m_device->DescriptorPoolsManager->AcquirePoolSetContainer();
}

#if GPU_ALLOW_PROFILE_EVENTS
void CmdBufferVulkan::BeginEvent(const char* name)
{
    if (!vkCmdBeginDebugUtilsLabelEXT)
        return;

    m_eventsBegin++;

    // Convert to ANSI
    char buffer[101];
    I32 i = 0;
    while (i < 100 && name[i])
    {
        buffer[i] = (char)name[i];
        i++;
    }
    buffer[i] = 0;

    VkDebugUtilsLabelEXT label{};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pLabelName = buffer;
    vkCmdBeginDebugUtilsLabelEXT(GetHandle(), &label);
}

void CmdBufferVulkan::EndEvent()
{
    if (m_eventsBegin == 0 || !vkCmdEndDebugUtilsLabelEXT)
        return;
    m_eventsBegin--;

    vkCmdEndDebugUtilsLabelEXT(GetHandle());
}
#endif

void CmdBufferVulkan::RefreshFenceStatus()
{
    if (m_state == State::Submitted)
    {
        auto fenceManager = m_fence->GetOwner();
        if (fenceManager->IsFenceSignaled(m_fence))
        {
            m_state = State::ReadyForBegin;

            m_submittedWaitSemaphores.clear();

            vkResetCommandBuffer(m_commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
            m_fence->GetOwner()->ResetFence(m_fence);
            m_fenceSignaledCounter++;

            if (m_descriptorPoolSetContainer)
            {
                m_device->DescriptorPoolsManager->ReleasePoolSet(*m_descriptorPoolSetContainer);
                m_descriptorPoolSetContainer = nullptr;
            }
        }
    }
    else
    {
        ASSERT(!m_fence->IsSignaled());
    }
}

CmdBufferVulkan::CmdBufferVulkan(GPUDeviceVulkan* device, CmdBufferPoolVulkan* pool)
    : m_device(device)
    , m_commandBuffer(VK_NULL_HANDLE)
    , m_state(State::ReadyForBegin)
    , m_fence(nullptr)
    , m_fenceSignaledCounter(0)
    , m_submittedFenceCounter(0)
    , m_commandBufferPool(pool)
{
    VkCommandBufferAllocateInfo createCmdBufInfo = vks::initializers::commandBufferAllocateInfo(m_commandBufferPool->GetHandle(),
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

    ThrowIfFailed(vkAllocateCommandBuffers(m_device->Device, &createCmdBufInfo, &m_commandBuffer));
    m_fence = m_device->FenceManager.AllocateFence();
}

CmdBufferVulkan::~CmdBufferVulkan()
{
    auto& fenceManager = m_device->FenceManager;
    if (m_state == State::Submitted)
    {
        // Wait 60ms
        const U64 waitForCmdBufferInNanoSeconds = 60 * 1000 * 1000LL;
        fenceManager.WaitAndReleaseFence(m_fence, waitForCmdBufferInNanoSeconds);
    }
    else
    {
        // Just free the fence, command buffer was not submitted
        fenceManager.ReleaseFence(m_fence);
    }

    vkFreeCommandBuffers(m_device->Device, m_commandBufferPool->GetHandle(), 1, &m_commandBuffer);
}

CmdBufferPoolVulkan::CmdBufferPoolVulkan(GPUDeviceVulkan* device)
    : m_device(device)
    , m_handle(VK_NULL_HANDLE)
{
}

CmdBufferPoolVulkan::~CmdBufferPoolVulkan()
{
    for (I32 i = 0; i < m_cmdBuffers.size(); ++i)
    {
        ::Delete(m_cmdBuffers[i]);
    }
    vkDestroyCommandPool(m_device->Device, m_handle, nullptr);
}

void CmdBufferPoolVulkan::RefreshFenceStatus(CmdBufferVulkan* skipCmdBuffer)
{
    for (I32 i = 0; i < m_cmdBuffers.size(); ++i)
    {
        auto cmdBuffer = m_cmdBuffers[i];
        if (cmdBuffer != skipCmdBuffer)
        {
            cmdBuffer->RefreshFenceStatus();
        }
    }
}

CmdBufferVulkan* CmdBufferPoolVulkan::Create()
{
    const auto cmdBuffer = New<CmdBufferVulkan>(m_device, this);
    m_cmdBuffers.push_back(cmdBuffer);
    return cmdBuffer;
}

void CmdBufferPoolVulkan::Create(U32 queueFamilyIndex)
{
    VkCommandPoolCreateInfo poolInfo = vks::initializers::commandPoolCreateInfo();
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    // TODO: use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT?
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ThrowIfFailed(vkCreateCommandPool(m_device->Device, &poolInfo, nullptr, &m_handle));
}

CmdBufferManagerVulkan::CmdBufferManagerVulkan(GPUDeviceVulkan* device, GPUContextVulkan* context)
    : m_device(device)
    , m_pool(device)
    , m_queue(context->GetQueue())
    , m_activeCmdBuffer(nullptr)
{
    m_pool.Create(m_queue->GetFamilyIndex());
}

void CmdBufferManagerVulkan::SubmitActiveCmdBuffer(SemaphoreVulkan* signalSemaphore)
{
    ASSERT(m_activeCmdBuffer);
    if (!m_activeCmdBuffer->IsSubmitted() && m_activeCmdBuffer->HasBegun())
    {
        if (m_activeCmdBuffer->IsInsideRenderPass())
        {
            m_activeCmdBuffer->EndRenderPass();
        }

        // Pause all active queries
        for (I32 i = 0; i < m_queriesInProgress.size(); i++)
        {
            m_queriesInProgress[i]->Interrupt(m_activeCmdBuffer);
        }

        m_activeCmdBuffer->End();

        if (signalSemaphore)
        {
            m_queue->Submit(m_activeCmdBuffer, signalSemaphore->GetHandle());
        }
        else
        {
            m_queue->Submit(m_activeCmdBuffer);
        }
    }

    m_activeCmdBuffer = nullptr;
}

void CmdBufferManagerVulkan::WaitForCmdBuffer(CmdBufferVulkan* cmdBuffer, float timeInSecondsToWait)
{
    ASSERT(cmdBuffer->IsSubmitted());
    const bool failed = m_device->FenceManager.WaitForFence(cmdBuffer->GetFence(), (U64)(timeInSecondsToWait * 1e9));
    ASSERT(!failed);
    cmdBuffer->RefreshFenceStatus();
}

void CmdBufferManagerVulkan::PrepareForNewActiveCommandBuffer()
{
    for (I32 i = 0; i < m_pool.m_cmdBuffers.size(); i++)
    {
        auto cmdBuffer = m_pool.m_cmdBuffers[i];
        cmdBuffer->RefreshFenceStatus();
        if (cmdBuffer->GetState() == CmdBufferVulkan::State::ReadyForBegin)
        {
            m_activeCmdBuffer = cmdBuffer;
            m_activeCmdBuffer->Begin();
            return;
        }
        else
        {
            ASSERT(cmdBuffer->GetState() == CmdBufferVulkan::State::Submitted);
        }
    }

    // Always begin fresh command buffer for rendering
    m_activeCmdBuffer = m_pool.Create();
    m_activeCmdBuffer->Begin();

    // Resume any paused queries with the new command buffer
    for (I32 i = 0; i < m_queriesInProgress.size(); i++)
    {
        m_queriesInProgress[i]->Resume(m_activeCmdBuffer);
    }
}

void CmdBufferManagerVulkan::OnQueryBegin(GPUTimerQueryVulkan* query)
{
    m_queriesInProgress.push_back(query);
}

void CmdBufferManagerVulkan::OnQueryEnd(GPUTimerQueryVulkan* query)
{
    U32 index = VectorFindIndex(m_queriesInProgress, query);
    m_queriesInProgress.erase(m_queriesInProgress.begin() + index);
}
