#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_PipelineLayout.h"
#include "Graphics/Descriptors.h"

#include <d3d12.h>
#include <d3dx12.h>

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::DX12
        {
            class RenderContext_DX12;

            struct RootSignatureParameters
            {
                std::vector<CD3DX12_ROOT_PARAMETER> RootParameters;
                std::vector<std::vector<CD3DX12_ROOT_DESCRIPTOR>> RootDescriptors;
                std::vector<std::vector<DescriptorType>> RootDescriptorTypes;
                std::vector<std::vector<DescriptorBinding>> DescriptorBinding;
                std::vector<std::vector<CD3DX12_DESCRIPTOR_RANGE>> DescriptorRanges;
            };

            // Define the max number of bindings a single set can have to be considered 
            // for root descriptors.
            constexpr u32 c_MaxRootDescriptorBindingForRootDescriptor = 1;

            class RHI_PipelineLayout_DX12 : public RHI_PipelineLayout
            {
            public:
                virtual ~RHI_PipelineLayout_DX12() override;

                ID3D12RootSignature* GetRootSignature() const;
                const RootSignatureParameters& GetRootSignatureParameters() const;

                // RHI_Pipeline
                virtual void Create(RenderContext* context, PipelineStateObject pso) override;
                virtual void Create(RenderContext* context, ComputePipelineStateObject pso) override;

                // RHI_Resource
                virtual void Release() override;
                virtual bool ValidResource() override;
                virtual void SetName(std::string name) override;

            private:
                void CreateLayout(RenderContext* context, RHI_Shader* shader, std::string_view name);

                bool CheckForRootDescriptors(const DescriptorSet& descriptorSet);
                std::vector<CD3DX12_ROOT_DESCRIPTOR> GetRootDescriptor(const DescriptorSet& descriptorSet);
                std::vector<CD3DX12_DESCRIPTOR_RANGE> GetDescriptoirRangesFromSet(const DescriptorSet& descriptorSet);

            private:
                ID3D12RootSignature* m_rootSignature = nullptr;
                RootSignatureParameters m_rootSignatureParameters;
                RenderContext_DX12* m_context;
            };
        }
    }
}
#endif // IS_VULKAN_ENABLED