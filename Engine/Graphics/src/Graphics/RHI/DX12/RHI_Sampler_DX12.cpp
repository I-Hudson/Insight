#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Sampler_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"

#include "Graphics/RHI/DX12/DX12Utils.h"

#include "Core/Profiler.h"
#include "Platforms/Platform.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			void RHI_SamplerManager_DX12::SetRenderContext(RenderContext* context)
			{
				m_context = static_cast<RenderContext_DX12*>(context);
			}

			RHI_Sampler* RHI_SamplerManager_DX12::GetOrCreateSampler(RHI_SamplerCreateInfo info)
			{
				const u64 hash = info.GetHash();
				if (auto itr = m_samplers.find(hash); itr != m_samplers.end())
				{
					return itr->second.Get();
				}

				std::array<float, 4> borderColour = BorderColourToDX12(info.BoarderColour);

				D3D12_SAMPLER_DESC desc = { };
				desc.Filter = FilterToDX12(info.MinFilter, info.CompareOp);
				desc.AddressU = SamplerAddressModelToDX12(info.AddressMode);
				desc.AddressV = SamplerAddressModelToDX12(info.AddressMode);
				desc.AddressW = SamplerAddressModelToDX12(info.AddressMode);
				desc.MipLODBias = info.MipLoadBias;
				desc.MaxAnisotropy = static_cast<UINT>(info.MaxAnisotropy);
				desc.ComparisonFunc = info.CompareEnabled ? CompareOpToDX12(info.CompareOp) : D3D12_COMPARISON_FUNC_NONE;
				Platform::MemCopy(&desc.BorderColor, borderColour.data(), sizeof(desc.BorderColor));
				desc.MinLOD = info.MinLod;
				desc.MaxLOD = info.MaxLod;

				DescriptorHeapHandle_DX12 handle = m_context->GetDescriptorHeap(DescriptorHeapTypes::Sampler).GetNewHandle();
				m_context->GetDevice()->CreateSampler(&desc, handle.CPUPtr);

				RHI_Sampler_DX12* new_sampler = ::New<RHI_Sampler_DX12, Insight::Core::MemoryAllocCategory::Graphics>();
				new_sampler->Handle = handle;
				new_sampler->Create_Info = info;

				m_samplers.emplace(hash, UPtr<RHI_Sampler>(new_sampler));
				return m_samplers.at(hash).Get();
			}

			void RHI_SamplerManager_DX12::ReleaseAll()
			{
				IS_PROFILE_FUNCTION();

				for (auto& pair : m_samplers)
				{
					RHI_Sampler_DX12* samplerDX12 = static_cast<RHI_Sampler_DX12*>(pair.second.Get());
					m_context->GetDescriptorHeap(DescriptorHeapTypes::Sampler).FreeHandle(samplerDX12->Handle);
				}
				m_samplers.clear();
			}
		}
	}
}
#endif // IS_DX12_ENABLED