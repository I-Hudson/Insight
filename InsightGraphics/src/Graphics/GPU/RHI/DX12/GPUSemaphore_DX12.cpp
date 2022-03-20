#include "Graphics/GPU/RHI/DX12/GPUSemaphore_DX12.h"
#include "Graphics/GPU/RHI/DX12/DX12Utils.h"
#include "Core/Logger.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			GPUSemaphore_DX12::~GPUSemaphore_DX12()
			{
				Destroy();
			}

			void GPUSemaphore_DX12::Signal()
			{
				IS_CORE_ERROR("[GPUSemaphore_DX12] DX12 does not support this.");
			}

			bool GPUSemaphore_DX12::IsSignaled() const
			{
				return m_semaphore->GetCompletedValue() == c_DX12SemaphoreSignal;
			}

			void GPUSemaphore_DX12::Wait()
			{
				if (!IsSignaled())
				{
					HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
					ThrowIfFailed(m_semaphore->SetEventOnCompletion(c_DX12SemaphoreSignal, eventHandle));
					WaitForSingleObject(eventHandle, INFINITE);
					CloseHandle(eventHandle);
				}
			}

			void GPUSemaphore_DX12::Create(bool signaled)
			{
				ThrowIfFailed(GetDevice()->GetDevice()->CreateFence(
					signaled ? c_DX12SemaphoreSignal : c_DX12SemaphoreNonSignal,
					D3D12_FENCE_FLAG_NONE,
					IID_PPV_ARGS(&m_semaphore)));
			}

			void GPUSemaphore_DX12::Destroy()
			{
				if (m_semaphore)
				{
					m_semaphore.Reset();
					m_semaphore = nullptr;
				}
			}
		}
	}
}