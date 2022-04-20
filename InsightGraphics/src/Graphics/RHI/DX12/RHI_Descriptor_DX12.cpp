#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			void RHI_DescriptorLayout_DX12::Release()
			{
				if (m_layout)
				{
					m_layout.Reset();
				}
			}

			/// <summary>
			/// RHI_DescriptorLayout_DX12
			/// </summary>
			/// <param name="context"></param>
			/// <param name="set"></param>
			/// <param name="descriptors"></param>
			void RHI_DescriptorLayout_DX12::Create(RenderContext* context, int set, std::vector<Descriptor> descriptors)
			{
				m_context = dynamic_cast<RenderContext_DX12*>(context);

				// Reference: https://github.com/shuhuai/DeferredShadingD3D12/blob/master/DeferredRender.cpp

				std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;
				ranges.reserve(descriptors.size());

				std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
				rootParameters.reserve(descriptors.size());

				int baseRegister = 0;
				for (const Descriptor& descriptor : descriptors)
				{
					CD3DX12_DESCRIPTOR_RANGE1 range = {};
					range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, baseRegister++);
					ranges.push_back(range);

					CD3DX12_ROOT_PARAMETER1 rootParameter = {};
					rootParameter.InitAsDescriptorTable(1, &ranges.back());
					rootParameters.push_back(rootParameter);
				}

				CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
				rootSignatureDesc.Init_1_1((UINT)rootParameters.size(), rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
				
				// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
				D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

				if (FAILED(m_context->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
				{
					featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
				}

				ComPtr<ID3DBlob> signature;
				ComPtr<ID3DBlob> error;
				ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
				ThrowIfFailed(m_context->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_layout)));
			}


			/// <summary>
			/// RHI_Descriptor_DX12
			/// </summary>
			void RHI_Descriptor_DX12::Release()
			{
			}
		}
	}
}