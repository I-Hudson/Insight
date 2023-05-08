#pragma once

#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/RenderContext.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_CommandList;
		class RHI_CommandList_DX12;

		namespace RHI::DX12
		{
			/// @brief Handle all process in relation to a dx queue.
			/// The main process being submission and signals.
			class RHI_Queue_DX12
			{
			public:

				void Initialise(ID3D12Device* device, GPUQueue gpuQueue);
				void Release();

				ID3D12CommandQueue* GetQueue() const;

				/// @brief Wait for the queue to finish any work.
				void Wait(u64 fenceValueToWait);

				void SignalAndWait();

				void Submit(const RHI_CommandList_DX12* cmdlist);

				/// @brief Submit work to the queue.
				/// @param cmdlist
				const u64 SubmitAndSignal(const RHI_CommandList_DX12* cmdlist);

				/// @brief Submit work to the queue and wait for it to be completed.
				/// @param cmdlist
				void SubmitAndWait(const RHI_CommandList_DX12* cmdlist);

				/// @brief Add a signal to the queue. 
				const u64 Signal();

			private:
				ID3D12Device* m_dxDevice = nullptr;
				ID3D12CommandQueue* m_dxQueue = nullptr;
				GPUQueue m_gpuQueue;

				FrameResource<ID3D12Fence*> m_dxSubmitFence;
				/// @brief Host side event.
				FrameResource<void*> m_submitFenceEvent;
				FrameResource<u64> m_submitFenceValue;
			};
		}
	}
}