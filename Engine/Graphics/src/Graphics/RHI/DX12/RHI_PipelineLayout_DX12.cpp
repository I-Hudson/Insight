#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_PipelineLayout_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"

#include "Graphics/RHI/DX12/RHI_Shader_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			RHI_PipelineLayout_DX12::~RHI_PipelineLayout_DX12()
			{
				Release();
			}

			ID3D12RootSignature* RHI_PipelineLayout_DX12::GetRootSignature() const
			{
				return m_rootSignature;
			}

			void RHI_PipelineLayout_DX12::Create(RenderContext* context, PipelineStateObject pso)
			{
				m_context = static_cast<RenderContext_DX12*>(context);

				const std::vector<DescriptorSet> descriptor_sets = pso.Shader->GetDescriptorSets();



				PushConstant push_constant = pso.Shader->GetPushConstant();


				SetName(pso.Name + "_Layout");
			}

			void RHI_PipelineLayout_DX12::Release()
			{
				if (m_rootSignature != nullptr)
				{
					m_rootSignature->Release();
					m_rootSignature = nullptr;
				}
			}

			bool RHI_PipelineLayout_DX12::ValidResource()
			{
				return m_rootSignature != nullptr;
			}

			void RHI_PipelineLayout_DX12::SetName(std::string name)
			{
				if (m_rootSignature)
				{
					m_context->SetObjectName(name, m_rootSignature);
				}
			}
		}
	}
}

#endif // IS_VULKAN_ENABLED