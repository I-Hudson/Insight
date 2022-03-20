#pragma once

#include "Graphics/GPU/GPUFence.h"
#include "Graphics/GPU/RHI/DX12/GPUDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class GPUFence_DX12 : public GPUFence, public GPUResource_DX12
			{
			public:
				GPUFence_DX12();
				virtual ~GPUFence_DX12() override;

				virtual void Wait() override;
				virtual void Reset() override;
				virtual bool IsSignaled() override;

				ID3D12Fence* GetFence() const { return m_fence.Get(); }

			protected:
				virtual void Create() override;
				virtual void Destroy() override;

			private:
				ComPtr<ID3D12Fence> m_fence;
				u64 m_fenceValue = 0;
			};
		}
	}
}