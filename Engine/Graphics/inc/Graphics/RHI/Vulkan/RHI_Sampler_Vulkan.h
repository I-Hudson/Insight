#pragma once

#include "Graphics/RHI/RHI_Sampler.h"

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;

		namespace RHI::Vulkan
		{
			class RenderContext_Vulkan;

			class RHI_SamplerManager_Vulkan : public RHI_SamplerManager
			{
			public:

				virtual void SetRenderContext(RenderContext* context) override;
				virtual RHI_Sampler* GetOrCreateSampler(RHI_SamplerCreateInfo info) override;
				virtual void ReleaseAll() override;

			private:
				RenderContext_Vulkan* m_context_vulkan = nullptr;
			};
		}
	}
}