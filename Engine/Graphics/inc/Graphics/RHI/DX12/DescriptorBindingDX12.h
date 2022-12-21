#pragma once

#ifdef IS_DX12_ENABLED

#include "Graphics/RHI/DX12/DescriptorTableDX12.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::DX12
        {
            class RHI_CommandList_DX12;
            class RHI_PipelineLayout_DX12;

            /// @brief Bind descriptors to the correct slots.
            class DescriptorBindingDX12
            {
            public:
                DescriptorBindingDX12();
                ~DescriptorBindingDX12();

                void SetPipeline(RHI_PipelineLayout_DX12* rootSignature);

                void SetConstantBufferView(u32 slot, RHI_BufferView view);
                void SetShaderResourceView(u32 slot, const RHI_Texture* texture);
                void SetUnorderedAccessView(u32 slot, const RHI_Resource* resource);

                void Bind(RHI_CommandList_DX12* cmdList);

            private:
                DescriptorTableDX12 m_table;
                RHI_PipelineLayout_DX12* m_rootSignature = nullptr;
                DescriptorHeapGPU_DX12 m_descriptorHeapRes;
            };
        }
    }
}
#endif // #ifdef IS_DX12_ENABLED