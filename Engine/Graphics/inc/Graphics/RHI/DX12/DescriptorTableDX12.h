#pragma once

#ifdef IS_DX12_ENABLED

#include "Core/TypeAlias.h"
#include "Graphics/RHI/DX12/RHI_PipelineLayout_DX12.h"
#include "Graphics/RHI/RHI_Buffer.h"

namespace Insight
{
    namespace Graphics
    {
        class RHI_Texture;

        namespace RHI::DX12
        {
            /// @brief Store a representation of a descriptor setup from a root signature.
            class DescriptorTableDX12
            {
            public:

                /// @brief Setup all required variables from a RootSignatureParameters.
                /// @param parameters 
                void SetupTable(const RootSignatureParameters& parameters);

                void SetConstantBufferView(u32 slot, RHI_BufferView view);
                void SetShaderResourceView(u32 slot, const RHI_Texture* texture);
                void SetUnorderedAccessView(u32 slot, const RHI_Resource* resource);

            private:
                std::vector<RHI_BufferView> m_tableCBVs;  // Store the buffers assigned to CBV in descriptor tables. Index is important.
                std::vector<const RHI_Texture*> m_tableSRVs; // Store the textures assigned to SRV in descriptor tables. Index is important.
                std::vector<const RHI_Resource*> m_tableUAVs; // Store the buffers assigned to UAV in descriptor tables. Index is important.

                std::vector<RHI_BufferView> m_rootCBVs;
                std::vector<const RHI_Texture*> m_rootSRVs;
                std::vector<const RHI_Resource*> m_rootUAVs;
            };
        }
    }
}
#endif // #ifdef IS_DX12_ENABLED