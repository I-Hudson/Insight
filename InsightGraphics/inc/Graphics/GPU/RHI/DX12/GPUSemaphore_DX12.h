#pragma once

#include "Graphics/GPU/GPUSemaphore.h"
#include "Graphics/GPU/RHI/DX12/GPUDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			const int c_DX12SemaphoreSignal = 1;
			const int c_DX12SemaphoreNonSignal = 0;

			class GPUSemaphore_DX12 : public GPUSemaphore, public GPUResource_DX12
			{
			public:
				virtual ~GPUSemaphore_DX12() override;

				virtual void Signal() override;

				bool IsSignaled() const;
				void Wait();

				ID3D12Fence* GetSemaphore() const { return m_semaphore.Get(); }

			protected:
				virtual void Create(bool signaled) override;
				virtual void Destroy() override;

			private:
				ComPtr<ID3D12Fence> m_semaphore{ nullptr };
			};
		}
	}
}