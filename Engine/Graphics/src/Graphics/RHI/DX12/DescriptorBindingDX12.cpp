#ifdef IS_DX12_ENABLED

#include "Graphics/RHI/DX12/DescriptorBindingDX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::DX12
        {
            DescriptorBindingDX12::DescriptorBindingDX12()
            {
                m_descriptorHeapRes.SetRenderContext(static_cast<RenderContext_DX12*>(&RenderContext::Instance()));
                m_descriptorHeapRes.Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }

            DescriptorBindingDX12::~DescriptorBindingDX12()
            {
                m_descriptorHeapRes.Destroy();
            }

            void DescriptorBindingDX12::SetPipeline(RHI_PipelineLayout_DX12* rootSignature)
            {
                m_rootSignature = rootSignature;
                m_table.SetupTable(rootSignature->GetRootSignatureParameters());
            }

            void DescriptorBindingDX12::SetConstantBufferView(u32 slot, RHI_BufferView view)
            {
                m_table.SetConstantBufferView(slot, view);
            }

            void DescriptorBindingDX12::SetShaderResourceView(u32 slot, const RHI_Texture* texture)
            {
                m_table.SetShaderResourceView(slot, texture);
            }

            void DescriptorBindingDX12::SetUnorderedAccessView(u32 slot, const RHI_Resource* resource)
            {
                m_table.SetUnorderedAccessView(slot, resource);
            }

            void DescriptorBindingDX12::Bind(RHI_CommandList_DX12* cmdList)
            {
                if (!m_rootSignature)
                {
                    return;
                }
                const RootSignatureParameters& rootParameters = m_rootSignature->GetRootSignatureParameters();
                for (size_t rootParamIdx = 0; rootParamIdx < rootParameters.RootParameters.size(); ++rootParamIdx)
                {
                    const CD3DX12_ROOT_PARAMETER& param = rootParameters.RootParameters.at(rootParamIdx);
                    if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
                    {
                        for (size_t rangeIdx = 0; rangeIdx < param.DescriptorTable.NumDescriptorRanges; ++rangeIdx)
                        {
                            const D3D12_DESCRIPTOR_RANGE& range = param.DescriptorTable.pDescriptorRanges[rangeIdx];
                            switch (range.RangeType)
                            {
                            case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                                FAIL_ASSERT();
                                break;

                            case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:

                                break;

                            case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                                FAIL_ASSERT();
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    else if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV)
                    {
                        FAIL_ASSERT();
                    }
                    else if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV)
                    {
                        FAIL_ASSERT();
                    }
                    else if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV)
                    {
                        FAIL_ASSERT();
                    }
                }
            }
        }
    }
}
#endif // #ifdef IS_DX12_ENABLED