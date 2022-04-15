#include "Graphics/RHI/Vulkan/PipelineStateObject_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Core/Logger.h"

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

			vk::PipelineLayout PipelineLayoutManager_Vulkan::GetOrCreateLayout()
			{
				assert(m_context != nullptr);

				const u64 hash = 0;
				auto itr = m_layouts.find(hash);
				if (itr != m_layouts.end())
				{
					return itr->second;
				}

				vk::PipelineLayoutCreateInfo createInfo = vk::PipelineLayoutCreateInfo(
					{},
					{},
					{});
				vk::PipelineLayout layout = m_context->GetDevice().createPipelineLayout(createInfo);
				m_layouts[hash] = layout;
				return layout;
			}

			void PipelineLayoutManager_Vulkan::Destroy()
			{
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
				assert(m_context != nullptr);

				const u64 psoHash = pso.GetHash();
				auto itr = m_pipelineStateObjects.find(psoHash);
				if (itr != m_pipelineStateObjects.end())
				{
					return itr->second;
				}

				// Create pipeline layout.
				// Create descriptor set layout
				vk::PipelineLayout layout = m_context->GetPipelineLayoutManager().GetOrCreateLayout();

				RHI_Shader_Vulkan* shaderVulkan = dynamic_cast<RHI_Shader_Vulkan*>(pso.Shader);
				std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;
				for (int i = 0; i < ShaderStage_Count; ++i)
				{
					vk::ShaderModule shaderModule = shaderVulkan->GetStage((ShaderStageFlagBits)i);
					if (!shaderModule)
					{
						continue;
					}
			
					vk::PipelineShaderStageCreateInfo info({}, ShaderStageFlagBitsToVulkan((ShaderStageFlagBits)i), shaderModule, "main");
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
					vk::FrontFace::eClockwise,										// frontFace
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
					vk::CompareOp::eLessOrEqual,                 // depthCompareOp
					false,                                       // depthBoundTestEnable
					false,                                       // stencilTestEnable
					stencilOpState,                              // front
					stencilOpState                               // back
				);
			
				vk::ColorComponentFlags colorComponentFlags = ColourComponentFlagsToVulkan(pso.ColourWriteMask);
				vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
					pso.BlendEnable ,									// blendEnable
					BlendFactorToVulkan(pso.SrcColourBlendFactor),		// srcColorBlendFactor
					BlendFactorToVulkan(pso.DstColourBlendFactor),		// dstColorBlendFactor
					BlendOpToVulkan(pso.ColourBlendOp),					// colorBlendOp
					BlendFactorToVulkan(pso.SrcAplhaBlendFactor),		// srcAlphaBlendFactor
					BlendFactorToVulkan(pso.DstAplhaBlendFactor),		// dstAlphaBlendFactor
					BlendOpToVulkan(pso.AplhaBlendOp),					// alphaBlendOp
					colorComponentFlags									// colorWriteMask
				);
			
				vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
					vk::PipelineColorBlendStateCreateFlags(),  // flags
					false,                                     // logicOpEnable
					vk::LogicOp::eNoOp,                        // logicOp
					pipelineColorBlendAttachmentState,         // attachments
					{ { 1.0f, 1.0f, 1.0f, 1.0f } }             // blendConstants
				);
			
				std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
				vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(), dynamicStates);
			
				vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
					vk::PipelineCreateFlags(),																	// flags
					pipelineShaderStageCreateInfos,																// stages
					&pipelineVertexInputStateCreateInfo.CreateInfo,												// pVertexInputState
					&pipelineInputAssemblyStateCreateInfo,														// pInputAssemblyState
					nullptr,																					// pTessellationState
					&pipelineViewportStateCreateInfo,															// pViewportState
					&pipelineRasterizationStateCreateInfo,														// pRasterizationState
					&pipelineMultisampleStateCreateInfo,														// pMultisampleState
					&pipelineDepthStencilStateCreateInfo,														// pDepthStencilState
					&pipelineColorBlendStateCreateInfo,															// pColorBlendState
					&pipelineDynamicStateCreateInfo,															// pDynamicState
					layout,																						// layout
					m_context->GetRenderpassManager().GetOrCreateRenderpass({ pso.RenderTargets })				// renderPass
				);

				vk::ResultValue<vk::Pipeline> result = m_context->GetDevice().createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
				m_pipelineStateObjects[psoHash] = result.value;

				return result.value;
			}

			void PipelineStateObjectManager_Vulkan::Destroy()
			{
				for (const auto pair : m_pipelineStateObjects)
				{
					m_context->GetDevice().destroyPipeline(pair.second);
				}
				m_pipelineStateObjects.clear();
			}


			RenderpassManager_Vulkan::~RenderpassManager_Vulkan()
			{
				if (m_renderpasses.size() > 0)
				{
					IS_CORE_ERROR("[GPURenderpassManager_Vulkan::~GPURenderpassManager_Vulkan] Not all renderpass have not been destroyed.");
				}
			}

			vk::RenderPass RenderpassManager_Vulkan::GetOrCreateRenderpass(RenderpassDesc_Vulkan desc)
			{
				assert(m_context != nullptr);

				const u64 hash = desc.GetHash();
				auto itr = m_renderpasses.find(hash);
				if (itr != m_renderpasses.end())
				{
					return itr->second;
				}

				std::vector<vk::AttachmentDescription> attachmentDescriptions;
				std::vector<vk::AttachmentReference> colorAttachments;
				vk::AttachmentReference depthAttachment;
				
				if (desc.RenderTargets.size() > 0)
				{
					for (const auto* rt : desc.RenderTargets)
					{
						// TODO: 
					}
				}
				else
				{
					
					// Assume this is a swapchain render pass as no user defined render targets are added.
					attachmentDescriptions.emplace_back(
						vk::AttachmentDescriptionFlags(),
						m_context->GetSwapchainColourFormat(),
						vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear,
						vk::AttachmentStoreOp::eStore,
						vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare,
						vk::ImageLayout::eUndefined,
						vk::ImageLayout::ePresentSrcKHR);
					colorAttachments.push_back(vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal));

					//attachmentDescriptions.emplace_back(vk::AttachmentDescriptionFlags(),
					//	vk::Format::eD16Unorm,
					//	vk::SampleCountFlagBits::e1,
					//	vk::AttachmentLoadOp::eClear,
					//	vk::AttachmentStoreOp::eDontCare,
					//	vk::AttachmentLoadOp::eDontCare,
					//	vk::AttachmentStoreOp::eDontCare,
					//	vk::ImageLayout::eUndefined,
					//	vk::ImageLayout::eDepthStencilAttachmentOptimal);
					//depthAttachment = vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				}

				vk::SubpassDescription  subpassDescription(
					vk::SubpassDescriptionFlags(),
					vk::PipelineBindPoint::eGraphics,
					{},
					colorAttachments,
					{},
					nullptr/*(depthAttachment.layout != vk::ImageLayout::eUndefined) ? &depthAttachment : nullptr*/);

				vk::RenderPass renderPass = m_context->GetDevice().createRenderPass(vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), attachmentDescriptions, subpassDescription));
				m_renderpasses[hash] = renderPass;
				return renderPass;
			}

			void RenderpassManager_Vulkan::Destroy()
			{
				for (const auto& pair : m_renderpasses)
				{
					m_context->GetDevice().destroyRenderPass(pair.second);
				}
				m_renderpasses.clear();
			}
		}
	}
}