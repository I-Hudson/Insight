#pragma once

#include "Core/TypeAlias.h"

#include <unordered_map>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;

		struct RHI_SamplerCreateInfo
		{
			u64 GetHash() const
			{
				u64 hash = 0;



				return hash;
			}
		};

		struct RHI_Sampler
		{
			void* Resource;
		};

		class RHI_SamplerManager
		{
		public:

			static RHI_SamplerManager* New();

			virtual void SetRenderContext(RenderContext* context) = 0;
			virtual RHI_Sampler GetOrCreateSampler(RHI_SamplerCreateInfo info) = 0;
			virtual void ReleaseAll() = 0;

		protected:
			std::unordered_map<u64, RHI_Sampler> m_samplers;
		};
	}
}