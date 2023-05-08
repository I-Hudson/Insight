#include "Graphics/RHI/DX12/RHI_Queue_DX12.h"
#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"

#include "Core/Logger.h"

#include <nvtx3/nvtx3.hpp>

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::DX12
        {
            void RHI_Queue_DX12::Initialise(ID3D12Device* device, GPUQueue gpuQueue)
            {
                m_dxDevice = device;
                m_gpuQueue = gpuQueue;

                if (!m_dxQueue)
                {
                    /// Describe and create the command queue.
                    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
                    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

                    switch (m_gpuQueue)
                    {
                    case Insight::Graphics::GPUQueue_Graphics:
                        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                        break;
                    case Insight::Graphics::GPUQueue_Compute:
                        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                        break;
                    case Insight::Graphics::GPUQueue_Transfer:
                        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
                        break;
                    default:
                        break;
                    }

                    HRESULT createCommandQueueResult = m_dxDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_dxQueue));
                    if (createCommandQueueResult != S_OK)
                    {
                        IS_CORE_WARN("[HI_Queue_DX12::Initialise] Queue not supported: {}, HR: {}",
                            (int)D3D12_COMMAND_LIST_TYPE_DIRECT, HrToString(createCommandQueueResult));
                    }
                }

                m_dxSubmitFence.Setup();
                m_dxSubmitFence.ForEach([this](ID3D12Fence*& fence)
                {
                    if (!fence)
                    {
                        m_dxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
                    }
                });

                m_submitFenceEvent.Setup();
                m_submitFenceEvent.ForEach([](void*& fence)
                {
                    if (!fence)
                    {
                        fence = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                    }
                });

                m_submitFenceValue.Setup();
                m_submitFenceValue.ForEach([](u64& fenceValue)
                {
                    fenceValue = 0;
                });
            }

            void RHI_Queue_DX12::Release()
            {
                if (m_dxQueue)
                {
                    m_dxQueue->Release();
                    m_dxQueue = nullptr;
                }

                m_dxSubmitFence.ForEach([](ID3D12Fence*& fence)
                {
                    if (fence)
                    {
                        fence->Release();
                        fence = nullptr;
                    }
                });

                m_submitFenceEvent.ForEach([](void*& fence)
                {
                    if (fence)
                    {
                        CloseHandle(fence);
                        fence = nullptr;
                    }
                });
            }

            ID3D12CommandQueue* RHI_Queue_DX12::GetQueue() const
            {
                return m_dxQueue;
            }

            void RHI_Queue_DX12::Wait(const u64 fenceValueToWait)
            {
                nvtx3::scoped_range range{ "RHI_Queue_DX12::Wait" };
                u64 fenceCompletedValue = m_dxSubmitFence.Get()->GetCompletedValue();
                if (fenceCompletedValue < fenceValueToWait)
                {
                    // Wait until the fence has been processed.
                    ThrowIfFailed(m_dxSubmitFence.Get()->SetEventOnCompletion(fenceValueToWait, m_submitFenceEvent.Get()));
                    WaitForSingleObjectEx(m_submitFenceEvent.Get(), INFINITE, FALSE);
                }
            }

            void RHI_Queue_DX12::SignalAndWait()
            {
                Wait(Signal());
            }

            void RHI_Queue_DX12::Submit(const RHI_CommandList_DX12* cmdlist)
            {
                nvtx3::scoped_range range{ "RHI_Queue_DX12::Submit" };          
                ID3D12CommandList* ppCommandLists[] = { cmdlist->GetCommandList() };
                m_dxQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
            }

            const u64 RHI_Queue_DX12::Signal()
            {
                nvtx3::scoped_range range{ "RHI_Queue_DX12::Signal" };

                // Increment the fence value for the current frame.
                u64 newFenceValue = ++m_submitFenceValue.Get();
                u64 currentFenceValue = m_dxSubmitFence.Get()->GetCompletedValue();

                // Schedule a Signal command in the queue.
                ThrowIfFailed(m_dxQueue->Signal(m_dxSubmitFence.Get(), newFenceValue));
                return newFenceValue;
            }

            const u64 RHI_Queue_DX12::SubmitAndSignal(const RHI_CommandList_DX12* cmdlist)
            {
                Submit(cmdlist);
                return Signal();
            }

            void RHI_Queue_DX12::SubmitAndWait(const RHI_CommandList_DX12* cmdlist)
            {
                nvtx3::scoped_range range{ "RHI_Queue_DX12::SubmitAndWait" };
                const u64 fenceValue = SubmitAndSignal(cmdlist);
                Wait(fenceValue);
            }
        }
    }
}