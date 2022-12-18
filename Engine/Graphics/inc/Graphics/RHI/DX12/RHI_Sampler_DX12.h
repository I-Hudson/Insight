#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_Sampler.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;

		namespace RHI::DX12
		{
			class RenderContext_DX12;

			class RHI_Sampler_DX12 : public RHI_Sampler
			{
			public:
				DescriptorHeapHandle_DX12 Handle;
			};

			class RHI_SamplerManager_DX12 : public RHI_SamplerManager
			{
			public:

				virtual void SetRenderContext(RenderContext* context) override;
				virtual RHI_Sampler* GetOrCreateSampler(RHI_SamplerCreateInfo info) override;
				virtual void ReleaseAll() override;

			private:
				RenderContext_DX12* m_context = nullptr;
			};
		}
	}
}
#endif // IS_DX12_ENABLED