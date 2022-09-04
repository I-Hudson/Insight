#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/PipelineStateObject_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"

#include "Graphics/RHI/Vulkan/VulkanUtils.h"

#include "Graphics/RenderTarget.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include "vulkan/vulkan.hpp"
#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			PipelineLayoutManager_Vulkan::PipelineLayoutManager_Vulkan()
			{ }

			PipelineLayoutManager_Vulkan::~PipelineLayoutManager_Vulkan()
			{ }

			vk::PipelineLayout PipelineLayoutManager_Vulkan::GetOrCreateLayout(PipelineStateObject pso)
			{
				IS_PROFILE_FUNCTION();
				assert(m_context != nullptr);

				const std::vector<Descriptor> descriptors = pso.Shader->GetDescriptors();
				u64 hash = 0;
				for (const Descriptor& descriptor : descriptors)
				{
					HashCombine(hash, descriptor.GetHash(false));
				}
				PushConstant push_constant = pso.Shader->GetPushConstant();
				HashCombine(hash, push_constant.ShaderStages);
				HashCombine(hash, push_constant.Offset);
				HashCombine(hash, push_constant.Size);

				auto itr = m_layouts.find(hash);
				if (itr != m_layouts.end())
				{
					return itr->second;
				}

				std::vector<vk::DescriptorSetLayout> setLayouts = {};

				int currentSet = 0;
				std::vector<Descriptor> currentDescriptors;

				for (const Descriptor& descriptor : descriptors)
				{
					if (currentSet != descriptor.Set)
					{
						RHI_DescriptorLayout_Vulkan* layoutVulkan = static_cast<RHI_DescriptorLayout_Vulkan*>(m_context->GetDescriptorLayoutManager().GetLayout(currentDescriptors));
						currentDescriptors.clear();
						currentSet = descriptor.Set;

						setLayouts.push_back(layoutVulkan->GetLayout());
					}
					currentDescriptors.push_back(descriptor);
				}
				RHI_DescriptorLayout_Vulkan* layoutVulkan = static_cast<RHI_DescriptorLayout_Vulkan*>(m_context->GetDescriptorLayoutManager().GetLayout(currentDescriptors));
				setLayouts.push_back(layoutVulkan->GetLayout());

				std::vector<vk::PushConstantRange> push_constants;
				if (push_constant.Size > 0)
				{
					push_constants.push_back(vk::PushConstantRange(ShaderStageFlagsToVulkan(push_constant.ShaderStages), push_constant.Offset, push_constant.Size));
				}

				vk::PipelineLayoutCreateInfo createInfo = vk::PipelineLayoutCreateInfo(
					{},
					setLayouts,
					push_constants);
				vk::PipelineLayout layout = m_context->GetDevice().createPipelineLayout(createInfo);
				m_layouts[hash] = layout;

				return layout;
			}

			void PipelineLayoutManager_Vulkan::Destroy()
			{
				IS_PROFILE_FUNCTION();

				for (const auto pair : m_layouts)
				{
					m_context->GetDevice().destroyPipelineLayout(pair.second);
				}
				m_layouts.clear();
			}

			PipelineStateObjectManager_Vulkan::PipelineStateObjectManager_Vulkan()
			{ }

			PipelineStateObjectManager_Vulkan::~PipelineStateObjectManager_Vulkan()
			{ }

			vk::Pipeline PipelineStateObjectManager_Vulkan::GetOrCreatePSO(PipelineStateObject pso)
			{
				IS_PROFILE_FUNCTION();

				assert(m_context != nullptr);

				const u64 psoHash = pso.GetHash();
				auto itr = m_pipelineStateObjects.find(psoHash);
				if (itr != m_pipelineStateObjects.end())
				{
					return itr->second;
				}

				// Create pipeline layout.
				// Create descriptor set layout
				vk::PipelineLayout layout = m_context->GetPipelineLayoutManager().GetOrCreateLayout(pso);

				RHI_Shader_Vulkan* shaderVulkan = static_cast<RHI_Shader_Vulkan*>(m_context->GetShaderManager().GetOrCreateShader(pso.ShaderDescription));
				std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;
				std::array<std::string, ShaderStageCount> shaderFuncNames;
				for (int i = 0; i < ShaderStageCount; ++i)
				{
					ShaderStageFlagBits shaderStage =static_cast<ShaderStageFlagBits>(1 << i);
					vk::ShaderModule shaderModule = shaderVulkan->GetStage(shaderStage);
					if (!shaderModule)
					{
						continue;
					}
			
					for (const wchar_t wChar : shaderVulkan->GetMainFuncName(shaderStage))
					{
						shaderFuncNames[i].push_back((char)wChar);
					}

					vk::PipelineShaderStageCreateInfo info({}, ShaderStageFlagBitsToVulkan(shaderStage),
						shaderModule, 
						shaderFuncNames[i].c_str());
					pipelineShaderStageCreateInfos.push_back(info);
				}
				const VertexInputLayout_Vulkan& pipelineVertexInputStateCreateInfo = shaderVulkan->GetVertexInputLayout();

				vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(),
					PrimitiveTopologyTypeToVulkan(pso.PrimitiveTopologyType));
			
				vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, nullptr, 1, nullptr);
			
				vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
					vk::PipelineRasterizationStateCreateFlags(),					// flags
					pso.DepthClampEnabled,											// depthClampEnable
					false,															// rasterizerDiscardEnable
					vk::PolygonMode::eFill,											// polygonMode
					CullModeToVulkan(pso.CullMode),									// cullMode
					FrontFaceToVulkan(pso.FrontFace),								// frontFace
					pso.DepthBaisEnabled,											// depthBiasEnable
					0.0f,															// depthBiasConstantFactor
					0.0f,															// depthBiasClamp
					0.0f,															// depthBiasSlopeFactor
					1.0f															// lineWidth
				);
			
				vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
					vk::PipelineMultisampleStateCreateFlags(),  // flags
					vk::SampleCountFlagBits::e1                 // rasterizationSamples
																// other values can be default
				);
			
				vk::StencilOpState stencilOpState(vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways);
				vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
					vk::PipelineDepthStencilStateCreateFlags(),  // flags
					pso.DepthTest,								 // depthTestEnable
					pso.DepthWrite,								 // depthWriteEnable
					CompareOpToVulkan(pso.DepthCompareOp),		 // depthCompareOp
					false,                                       // depthBoundTestEnable
					false,                                       // stencilTestEnable
					stencilOpState,                              // front
					stencilOpState                               // back
				);
			
				vk::ColorComponentFlags colorComponentFlags = ColourComponentFlagsToVulkan(pso.ColourWriteMask);
				std::vector<vk::PipelineColorBlendAttachmentState> pipeline_colour_blend_attachment_states;
				if (pso.Swapchain)
				{
					vk::PipelineColorBlendAttachmentState blend_state(
						pso.BlendEnable,									// blendEnable
						BlendFactorToVulkan(pso.SrcColourBlendFactor),		// srcColorBlendFactor
						BlendFactorToVulkan(pso.DstColourBlendFactor),		// dstColorBlendFactor
						BlendOpToVulkan(pso.ColourBlendOp),					// colorBlendOp
						BlendFactorToVulkan(pso.SrcAplhaBlendFactor),		// srcAlphaBlendFactor
						BlendFactorToVulkan(pso.DstAplhaBlendFactor),		// dstAlphaBlendFactor
						BlendOpToVulkan(pso.AplhaBlendOp),					// alphaBlendOp
						colorComponentFlags									// colorWriteMask
					);
					pipeline_colour_blend_attachment_states.push_back(blend_state);
				}
				else
				{
					for (const RHI_Texture* tex : pso.RenderTargets)
					{
						if (tex)
						{
							vk::PipelineColorBlendAttachmentState blend_state(
								pso.BlendEnable,									// blendEnable
								BlendFactorToVulkan(pso.SrcColourBlendFactor),		// srcColorBlendFactor
								BlendFactorToVulkan(pso.DstColourBlendFactor),		// dstColorBlendFactor
								BlendOpToVulkan(pso.ColourBlendOp),					// colorBlendOp
								BlendFactorToVulkan(pso.SrcAplhaBlendFactor),		// srcAlphaBlendFactor
								BlendFactorToVulkan(pso.DstAplhaBlendFactor),		// dstAlphaBlendFactor
								BlendOpToVulkan(pso.AplhaBlendOp),					// alphaBlendOp
								colorComponentFlags									// colorWriteMask
							);
							pipeline_colour_blend_attachment_states.push_back(blend_state);
						}
					}
				}

				vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
					vk::PipelineColorBlendStateCreateFlags(),  // flags
					false,                                     // logicOpEnable
					vk::LogicOp::eNoOp,                        // logicOp
					pipeline_colour_blend_attachment_states,   // attachments
					{ { 1.0f, 1.0f, 1.0f, 1.0f } }             // blendConstants
				);
			
				std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
				vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(), dynamicStates);
			
				RenderContext* context = (RenderContext*)m_context;
				RHI_Renderpass rhiRenderpass = context->GetRenderpassManager().GetRenderpass(pso.Renderpass);
				vk::RenderPass renderpass = *reinterpret_cast<vk::RenderPass*>(&rhiRenderpass);

				vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
					vk::PipelineCreateFlags(),																						// flags
					pipelineShaderStageCreateInfos,																					// stages
					&pipelineVertexInputStateCreateInfo.CreateInfo,																	// pVertexInputState
					&pipelineInputAssemblyStateCreateInfo,																			// pInputAssemblyState
					nullptr,																										// pTessellationState
					&pipelineViewportStateCreateInfo,																				// pViewportState
					&pipelineRasterizationStateCreateInfo,																			// pRasterizationState
					&pipelineMultisampleStateCreateInfo,																			// pMultisampleState
					&pipelineDepthStencilStateCreateInfo,																			// pDepthStencilState
					&pipelineColorBlendStateCreateInfo,																				// pColorBlendState
					&pipelineDynamicStateCreateInfo,																				// pDynamicState
					layout,																											// layout
					renderpass																										// renderPass
				);

				if (pso.Swapchain)
				{
					pso.RenderTargets[0] = m_context->GetSwaphchainIamge();
				}

				std::vector<vk::Format> colourAttachmentFormats;
				vk::Format depthAttachmentFormat = vk::Format::eUndefined;
				vk::Format stencilAttachmentFormat = vk::Format::eUndefined;
				for (const RHI_Texture* tex : pso.RenderTargets)
				{
					if (tex)
					{
						colourAttachmentFormats.push_back(PixelFormatToVkFormat[(int)tex->GetFormat()]);
					}
				}
				if (pso.DepthStencil)
				{
					depthAttachmentFormat = PixelFormatToVkFormat[(int)pso.DepthStencil->GetFormat()];
				}

				vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo = { };
				pipelineRenderingCreateInfo.setColorAttachmentFormats(colourAttachmentFormats);
				pipelineRenderingCreateInfo.setDepthAttachmentFormat(depthAttachmentFormat);
				pipelineRenderingCreateInfo.setStencilAttachmentFormat(stencilAttachmentFormat);

				if (m_context->IsExtensionEnabled(DeviceExtension::VulkanDynamicRendering))
				{
					graphicsPipelineCreateInfo.setPNext(&pipelineRenderingCreateInfo);
					graphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE;
				}

				vk::ResultValue<vk::Pipeline> result = m_context->GetDevice().createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
				m_pipelineStateObjects[psoHash] = result.value;

				return result.value;
			}

			void PipelineStateObjectManager_Vulkan::Destroy()
			{
				IS_PROFILE_FUNCTION();

				for (const auto pair : m_pipelineStateObjects)
				{
					m_context->GetDevice().destroyPipeline(pair.second);
				}
				m_pipelineStateObjects.clear();
			}
		}
	}
}

#endif //#if defined(IS_VULKAN_ENABLED)