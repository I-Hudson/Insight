#ifdef IS_DX12_ENABLED

#include "Graphics/RHI/DX12/DescriptorTableDX12.h"
namespace Insight
{
    namespace Graphics
    {
        namespace RHI::DX12
        {
            void DescriptorTableDX12::SetupTable(const RootSignatureParameters& parameters)
            {
                m_tableCBVs.clear();
                m_tableSRVs.clear();
                m_tableUAVs.clear();
                m_rootCBVs.clear();
                m_rootSRVs.clear();
                m_rootUAVs.clear();

                for (size_t i = 0; i < parameters.RootParameters.size(); ++i)
                {
                    const CD3DX12_ROOT_PARAMETER& param = parameters.RootParameters.at(i);
                    if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
                    {
                        for (size_t i = 0; i < param.DescriptorTable.NumDescriptorRanges; ++i)
                        {
                            const D3D12_DESCRIPTOR_RANGE& range = param.DescriptorTable.pDescriptorRanges[i];
                            switch (range.RangeType)
                            {
                            case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                                m_tableSRVs.push_back(nullptr);
                                m_rootSRVs.push_back(nullptr);
                                break;
                            case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                                m_tableCBVs.push_back({});
                                m_rootCBVs.push_back({});
                                break;
                            case D3D12_ROOT_PARAMETER_TYPE_UAV:
                                m_tableUAVs.push_back(nullptr);
                                m_rootUAVs.push_back(nullptr);
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    else if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV)
                    {
                        m_tableSRVs.push_back(nullptr);
                        m_rootSRVs.push_back(nullptr);
                    }
                    else if (param.ParameterType == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
                    {
                        m_tableCBVs.push_back({});
                        m_rootCBVs.push_back({});
                    }
                    else if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV)
                    {
                        m_tableUAVs.push_back(nullptr);
                        m_rootUAVs.push_back(nullptr);
                    }
                }
            }

            void DescriptorTableDX12::SetConstantBufferView(u32 slot, RHI_BufferView view)
            {
                m_tableCBVs.at(slot) = view;
                m_rootCBVs.at(slot) = view;
            }

            void DescriptorTableDX12::SetShaderResourceView(u32 slot, const RHI_Texture* texture)
            {
                m_tableSRVs.at(slot) = texture;
                m_rootSRVs.at(slot) = texture;
            }

            void DescriptorTableDX12::SetUnorderedAccessView(u32 slot, const RHI_Resource* resource)
            {
                m_tableUAVs.at(slot) = resource;
                m_rootUAVs.at(slot) = resource;
            }
        }
    }
}
#endif // #ifdef IS_DX12_ENABLED