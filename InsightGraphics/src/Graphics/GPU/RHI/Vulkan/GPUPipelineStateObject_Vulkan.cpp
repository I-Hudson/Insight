#include "Graphics/GPU/RHI/Vulkan/GPUPipelineStateObject_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUShader_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/GPU/GPUSwapchain.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			GPUPipelineStateObject_Vulkan::GPUPipelineStateObject_Vulkan()
			{
			}

			GPUPipelineStateObject_Vulkan::~GPUPipelineStateObject_Vulkan()
			{
				Destroy();
			}

			void GPUPipelineStateObject_Vulkan::Create(PipelineStateObject pso)
			{
				m_pso = pso;

				// Create descriptor set layout
			
				m_pipelineLayout = GetDevice()->GetDevice().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, {}, {}));

				GPUShader_Vulkan* shaderVulkan = dynamic_cast<GPUShader_Vulkan*>(m_pso.Shader);
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
					PrimitiveTopologyTypeToVulkan(m_pso.PrimitiveTopologyType));

				vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, nullptr, 1, nullptr);

				vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
					vk::PipelineRasterizationStateCreateFlags(),					// flags
					m_pso.DepthClampEnabled,                                        // depthClampEnable
					false,															// rasterizerDiscardEnable
					vk::PolygonMode::eFill,											// polygonMode
					vk::CullModeFlagBits::eBack,									// cullMode
					vk::FrontFace::eClockwise,										// frontFace
					m_pso.DepthBaisEnabled,											// depthBiasEnable
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
					m_pso.DepthTest,                             // depthTestEnable
					m_pso.DepthWrite,                            // depthWriteEnable
					vk::CompareOp::eLessOrEqual,                 // depthCompareOp
					false,                                       // depthBoundTestEnable
					false,                                       // stencilTestEnable
					stencilOpState,                              // front
					stencilOpState                               // back
				);

				vk::ColorComponentFlags colorComponentFlags = ColourComponentFlagsToVulkan(m_pso.ColourWriteMask);
				vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
					m_pso.BlendEnable ,									// blendEnable
					BlendFactorToVulkan(m_pso.SrcColourBlendFactor),	// srcColorBlendFactor
					BlendFactorToVulkan(m_pso.DstColourBlendFactor),	// dstColorBlendFactor
					BlendOpToVulkan(m_pso.ColourBlendOp),				// colorBlendOp
					BlendFactorToVulkan(m_pso.SrcAplhaBlendFactor),		// srcAlphaBlendFactor
					BlendFactorToVulkan(m_pso.DstAplhaBlendFactor),		// dstAlphaBlendFactor
					BlendOpToVulkan(m_pso.AplhaBlendOp),       // alphaBlendOp
					colorComponentFlags      // colorWriteMask
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
					m_pipelineLayout,																			// layout
					GPURenderpassManager_Vulkan::Instance().GetOrCreateRenderpass({ m_pso.RenderTargets })		// renderPass
				);
				vk::ResultValue<vk::Pipeline> result = GetDevice()->GetDevice().createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
				m_pipeline = result.value;
			}

			void GPUPipelineStateObject_Vulkan::Destroy()
			{
				if (m_pipeline)
				{
					GetDevice()->GetDevice().destroyPipeline(m_pipeline);
					m_pipeline = nullptr;
				}

				if (m_pipelineLayout)
				{
					GetDevice()->GetDevice().destroyPipelineLayout(m_pipelineLayout);
					m_pipelineLayout = nullptr;
				}
			}



			GPURenderpassManager_Vulkan::~GPURenderpassManager_Vulkan()
			{
				if (m_renderpasses.size() > 0)
				{
					std::cout << "[GPURenderpassManager_Vulkan::~GPURenderpassManager_Vulkan] Destroy must be called.\n";
				}
			}

			vk::RenderPass GPURenderpassManager_Vulkan::GetOrCreateRenderpass(GPURenderpassDesc_Vulkan desc)
			{
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
						PixelFormatToVulkan(GetDevice()->GetSwapchain()->GetColourFormat()),
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

				vk::RenderPass renderPass = GetDevice()->GetDevice().createRenderPass(vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), attachmentDescriptions, subpassDescription));
				m_renderpasses[hash] = renderPass;
				return renderPass;
			}

			void GPURenderpassManager_Vulkan::Destroy()
			{
				for (const auto& pair : m_renderpasses)
				{
					GetDevice()->GetDevice().destroyRenderPass(pair.second);
				}
				m_renderpasses.clear();
			}
}
	}
}