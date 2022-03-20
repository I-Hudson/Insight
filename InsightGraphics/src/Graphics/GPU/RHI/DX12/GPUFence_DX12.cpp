#include "Graphics/GPU/RHI/DX12/GPUFence_DX12.h"
#include "Graphics/GPU/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			GPUFence_DX12::GPUFence_DX12()
			{
			}

			GPUFence_DX12::~GPUFence_DX12()
			{
				Destroy();
			}

			void GPUFence_DX12::Wait()
			{
				const UINT64 cValue = m_fence->GetCompletedValue();
				if (cValue < m_fenceValue)
				{
					++m_fenceValue;
					HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
					ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, eventHandle));
					WaitForSingleObject(eventHandle, INFINITE);
					CloseHandle(eventHandle);
				}
			}

			void GPUFence_DX12::Reset()
			{
				++m_fenceValue;
				ID3D12CommandQueue* queue = GetDevice()->GetQueue(GPUQueue_Graphics);
				ThrowIfFailed(queue->Signal(GetFence(), m_fenceValue));
			}

			bool GPUFence_DX12::IsSignaled()
			{
				return !(m_fence->GetCompletedValue() < m_fenceValue);
			}

			void GPUFence_DX12::Create()
			{
				ThrowIfFailed(GetDevice()->GetDevice()->CreateFence(
					0, 
					D3D12_FENCE_FLAG_NONE, 
					IID_PPV_ARGS(&m_fence)));
			}

			void GPUFence_DX12::Destroy()
			{
				if (m_fence)
				{
					m_fence.Reset();
					m_fence = nullptr;
				}
			}
		}
	}
}