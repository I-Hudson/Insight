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

			const RootSignatureParameters& RHI_PipelineLayout_DX12::GetRootSignatureParameters() const
			{
				return m_rootSignatureParameters;
			}

			void RHI_PipelineLayout_DX12::Create(RenderContext* context, PipelineStateObject pso)
			{
				m_context = static_cast<RenderContext_DX12*>(context);

				const u64 RootSignitureMaxSlots = 64;
				u64 RootSignitureCurrentSlotsUsed = 0;
				const std::vector<DescriptorSet> descriptor_sets = pso.Shader->GetDescriptorSets();
				m_rootSignatureParameters = {};

				u32 rootParamterIdx = 0;
				for (const DescriptorSet& set : descriptor_sets)
				{
					if (CheckForRootDescriptors(set))
					{
						m_rootSignatureParameters.DescriptorRanges.push_back({});
						// Root Descriptors
						m_rootSignatureParameters.RootDescriptors.push_back(GetRootDescriptor(set));
						std::vector<DescriptorType> descriptorTypes;
						for (size_t i = 0; i < set.Bindings.size(); ++i)
						{
							descriptorTypes.push_back(set.Bindings.at(i).Type);
						}
						m_rootSignatureParameters.RootDescriptorTypes.push_back(descriptorTypes);
						RootSignitureCurrentSlotsUsed += 2;
					}
					else
					{
						m_rootSignatureParameters.RootDescriptors.push_back({});
						m_rootSignatureParameters.RootDescriptorTypes.push_back({});
						// Root Tables
						m_rootSignatureParameters.DescriptorRanges.push_back(GetDescriptoirRangesFromSet(set));
						RootSignitureCurrentSlotsUsed += 1;
					}
				}

				ASSERT(RootSignitureCurrentSlotsUsed < RootSignitureMaxSlots);

				// Create our descriptor definitions.
				u32 rootParameterIdx = 0;
				for (const DescriptorSet& set : descriptor_sets)
				{
					if (m_rootSignatureParameters.RootDescriptors.at(rootParameterIdx).size() > 0)
					{
						std::vector<DescriptorType> const& descriptorTypes = m_rootSignatureParameters.RootDescriptorTypes.at(rootParameterIdx);
						u32 rootIdx = 0;

						for (CD3DX12_ROOT_DESCRIPTOR const& root : m_rootSignatureParameters.RootDescriptors.at(rootParameterIdx))
						{
							DescriptorType descriptorType = descriptorTypes.at(rootIdx);
							CD3DX12_ROOT_PARAMETER paramter;

							switch (descriptorType)
							{
							case DescriptorType::Unifom_Buffer:
							{
								paramter.InitAsConstantBufferView(root.ShaderRegister, root.RegisterSpace);
								break;
							}
							case DescriptorType::Sampled_Image:
							{
								paramter.InitAsShaderResourceView(root.ShaderRegister, root.RegisterSpace);
								break;
							}
							case DescriptorType::Storage_Buffer:
							case DescriptorType::Storage_Image:
							{
								paramter.InitAsUnorderedAccessView(root.ShaderRegister, root.RegisterSpace);
								break;
							}
							}

							m_rootSignatureParameters.RootParameters.push_back(paramter);
							++rootIdx;
						}
					}
					else
					{
						std::vector<CD3DX12_DESCRIPTOR_RANGE> const& range = m_rootSignatureParameters.DescriptorRanges.at(rootParameterIdx);
						CD3DX12_ROOT_PARAMETER paramter;
						paramter.InitAsDescriptorTable(
							static_cast<UINT>(range.size()), range.data());
						m_rootSignatureParameters.RootParameters.push_back(paramter);
					}
					++rootParameterIdx;
				}

				CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC  signatureDesc(
					static_cast<UINT>(m_rootSignatureParameters.RootParameters.size()),
					m_rootSignatureParameters.RootParameters.data(),
					0,
					nullptr,
					D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
				);

				ID3DBlob* pSerializedRootSig;
				ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &pSerializedRootSig, nullptr));

				m_context->GetDevice()->CreateRootSignature(0, pSerializedRootSig->GetBufferPointer(), pSerializedRootSig->GetBufferSize(),
					IID_PPV_ARGS(&m_rootSignature));
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

			bool RHI_PipelineLayout_DX12::CheckForRootDescriptors(const DescriptorSet& descriptorSet)
			{
				for (const DescriptorBinding& binding : descriptorSet.Bindings)
				{
					if (binding.Type != DescriptorType::Unifom_Buffer
						&& binding.Type != DescriptorType::Sampled_Image
						&& binding.Type != DescriptorType::Storage_Buffer
						&& binding.Type != DescriptorType::Storage_Image)
					{
						return false;
					}
				}
				return descriptorSet.Bindings.size() <= c_MaxRootDescriptorBindingForRootDescriptor;
			}

			std::vector<CD3DX12_ROOT_DESCRIPTOR> RHI_PipelineLayout_DX12::GetRootDescriptor(const DescriptorSet& descriptorSet)
			{
				std::vector<CD3DX12_ROOT_DESCRIPTOR> rootDescriptors;
				for (const DescriptorBinding& binding : descriptorSet.Bindings)
				{
					if (binding.Type == DescriptorType::Unifom_Buffer
						|| binding.Type == DescriptorType::Sampled_Image
						|| binding.Type == DescriptorType::Storage_Buffer
						|| binding.Type == DescriptorType::Storage_Image)
					{
						rootDescriptors.push_back(CD3DX12_ROOT_DESCRIPTOR(binding.Binding, binding.Set));
					}
				}
				return rootDescriptors;
			}

			std::vector<CD3DX12_DESCRIPTOR_RANGE> RHI_PipelineLayout_DX12::GetDescriptoirRangesFromSet(const DescriptorSet& descriptorSet)
			{
				bool isSamplerTable = false;
				std::vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges;
				for (const DescriptorBinding& binding : descriptorSet.Bindings)
				{
					// Ensure samplers are not part of set with other descriptor types.
					if (!isSamplerTable)
					{
						if (descriptorRanges.size() == 0
							&& binding.Type == DescriptorType::Sampler)
						{
							isSamplerTable = true;
						}
						else
						{
							ASSERT(binding.Type != DescriptorType::Sampler);
						}
					}
					else
					{
						ASSERT(binding.Type == DescriptorType::Sampler);
					}

					descriptorRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(
						DescriptorRangeTypeToDX12(binding.Type), 
						1,
						binding.Binding,
						binding.Set));
				}
				return descriptorRanges;
			}
		}
	}
}

#endif // IS_VULKAN_ENABLED