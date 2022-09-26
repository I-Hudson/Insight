#pragma once

#include "Graphics/Enums.h"

#include "Core/TypeAlias.h"
#include "Core/Memory.h"

#include <unordered_map>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;

		struct RHI_SamplerCreateInfo
		{
			Filter MagFilter = Filter::Linear;
			Filter MinFilter = Filter::Linear;
			SamplerMipmapMode MipmapMode = SamplerMipmapMode::Linear;
			SamplerAddressMode AddressMode = SamplerAddressMode::MirroredRepeat;
			float MipLoadBias = 0.0f;
			bool AnisotropyEnabled = false;
			float MaxAnisotropy = 1.0f;
			bool CompareEnabled = false;
			CompareOp CompareOp = CompareOp::Less;
			float MinLod = 0.0f;
			float MaxLod = 0.0f;
			BorderColour BoarderColour = BorderColour::FloatOpaqueWhite;
			bool UnnormalisedCoordinates = false;

			u64 GetHash() const
			{
				u64 hash = 0;

				HashCombine(hash, MagFilter);
				HashCombine(hash, MinFilter);
				HashCombine(hash, MipmapMode);
				HashCombine(hash, AddressMode);
				HashCombine(hash, MipLoadBias);
				HashCombine(hash, AnisotropyEnabled);
				HashCombine(hash, MaxAnisotropy);
				HashCombine(hash, CompareEnabled);
				HashCombine(hash, CompareOp);
				HashCombine(hash, MinLod);
				HashCombine(hash, MaxLod);
				HashCombine(hash, BoarderColour);
				HashCombine(hash, UnnormalisedCoordinates);

				return hash;
			}
		};

		class RHI_Sampler
		{
		public:

			RHI_SamplerCreateInfo Create_Info = { };
			void* Resource = nullptr;
		};

		class RHI_SamplerManager
		{
		public:

			static RHI_SamplerManager* New();

			virtual void SetRenderContext(RenderContext* context) = 0;
			virtual RHI_Sampler* GetOrCreateSampler(RHI_SamplerCreateInfo info) = 0;
			virtual void ReleaseAll() = 0;

		protected:
			std::unordered_map<u64, UPtr<RHI_Sampler>> m_samplers;
		};
	}
}