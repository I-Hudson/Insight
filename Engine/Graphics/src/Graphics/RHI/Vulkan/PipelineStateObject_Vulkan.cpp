#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/PipelineStateObject_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"

#include "Graphics/RHI/Vulkan/VulkanUtils.h"

#include "Graphics/RenderTarget.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include <vulkan/vulkan.h>

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

			VkPipelineLayout PipelineLayoutManager_Vulkan::GetOrCreateLayout(PipelineStateObject pso)
			{
				IS_PROFILE_FUNCTION();
				assert(m_context != nullptr);

				const std::vector<DescriptorSet> descriptor_sets = pso.Shader->GetDescriptorSets();
				u64 hash = 0;
				for (const DescriptorSet& descriptor_set : descriptor_sets)
				{
					HashCombine(hash, descriptor_set.GetHash(false));
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

				std::vector<VkDescriptorSetLayout> set_layouts = {};
				std::vector<DescriptorSet> current_descriptor_sets;

				for (const DescriptorSet& descriptor_set : descriptor_sets)
				{
					RHI_DescriptorLayout_Vulkan* layoutVulkan = static_cast<RHI_DescriptorLayout_Vulkan*>(m_context->GetDescriptorLayoutManager().GetLayout(descriptor_set));
					set_layouts.push_back(layoutVulkan->GetLayout());
				}

				std::vector<VkPushConstantRange> push_constants;
				if (push_constant.Size > 0)
				{
					VkPushConstantRange pushConstantRange = {};
					pushConstantRange.stageFlags = ShaderStageFlagsToVulkan(push_constant.ShaderStages);
					pushConstantRange.offset = push_constant.Offset;
					pushConstantRange.size = push_constant.Size;
					push_constants.push_back(pushConstantRange);
				}

				VkPipelineLayoutCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				createInfo.pSetLayouts = set_layouts.data();
				createInfo.setLayoutCount = static_cast<u32>(set_layouts.size());
				createInfo.pPushConstantRanges = push_constants.data();
				createInfo.pushConstantRangeCount = static_cast<u32>(push_constants.size());

				VkPipelineLayout layout;
				ThrowIfFailed(vkCreatePipelineLayout(m_context->GetDevice(), &createInfo, nullptr, &layout));
				m_layouts[hash] = layout;

				return layout;
			}

			void PipelineLayoutManager_Vulkan::Destroy()
			{
				IS_PROFILE_FUNCTION();

				for (const auto pair : m_layouts)
				{
					vkDestroyPipelineLayout(m_context->GetDevice(), pair.second, nullptr);
				}
				m_layouts.clear();
			}

			PipelineStateObjectManager_Vulkan::PipelineStateObjectManager_Vulkan()
			{ }

			PipelineStateObjectManager_Vulkan::~PipelineStateObjectManager_Vulkan()
			{ }

			VkPipeline PipelineStateObjectManager_Vulkan::GetOrCreatePSO(PipelineStateObject pso)
			{
				IS_PROFILE_FUNCTION();

				assert(m_context != nullptr);

				const u64 psoHash = pso.GetHash();
				auto itr = m_pipelineStateObjects.find(psoHash);
				if (itr != m_pipelineStateObjects.end())
				{
					return itr->second;
				}

				/// Create pipeline layout.
				/// Create descriptor set layout
				VkPipelineLayout layout = m_context->GetPipelineLayoutManager().GetOrCreateLayout(pso);

				RHI_Shader_Vulkan* shaderVulkan = static_cast<RHI_Shader_Vulkan*>(m_context->GetShaderManager().GetOrCreateShader(pso.ShaderDescription));
				std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;
				std::array<std::string, ShaderStageCount> shaderFuncNames;
				for (int i = 0; i < ShaderStageCount; ++i)
				{
					ShaderStageFlagBits shaderStage =static_cast<ShaderStageFlagBits>(1 << i);
					VkShaderModule shaderModule = shaderVulkan->GetStage(shaderStage);
					if (!shaderModule)
					{
						continue;
					}
			
					for (const wchar_t wChar : shaderVulkan->GetMainFuncName(shaderStage))
					{
						shaderFuncNames[i].push_back((char)wChar);
					}

					VkPipelineShaderStageCreateInfo info = {};
					info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					info.stage = ShaderStageFlagBitsToVulkan(shaderStage);
					info.module = shaderModule;
					info.pName = shaderFuncNames[i].c_str();
					pipelineShaderStageCreateInfos.push_back(info);
				}
				const VertexInputLayout_Vulkan& pipelineVertexInputStateCreateInfo = shaderVulkan->GetVertexInputLayout();

				VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
				pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				pipelineInputAssemblyStateCreateInfo.topology = PrimitiveTopologyTypeToVulkan(pso.PrimitiveTopologyType);

				VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
				pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				pipelineViewportStateCreateInfo.viewportCount = 1;
				pipelineViewportStateCreateInfo.pViewports = nullptr;
				pipelineViewportStateCreateInfo.scissorCount = 1;
				pipelineViewportStateCreateInfo.pScissors = nullptr;
			
				VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
				pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				pipelineRasterizationStateCreateInfo.depthClampEnable = pso.DepthClampEnabled;
				pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = false;
				pipelineRasterizationStateCreateInfo.polygonMode = PolygonModeToVulkan(pso.PolygonMode);
				pipelineRasterizationStateCreateInfo.cullMode = CullModeToVulkan(pso.CullMode);
				pipelineRasterizationStateCreateInfo.frontFace = FrontFaceToVulkan(pso.FrontFace);
				pipelineRasterizationStateCreateInfo.depthBiasEnable = pso.DepthBaisEnabled;
				pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
				pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
				pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
				pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
			
				VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {};
				pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			
				VkStencilOpState stencilOpState = {};
				stencilOpState.failOp = VK_STENCIL_OP_KEEP;
				stencilOpState.passOp = VK_STENCIL_OP_KEEP;
				stencilOpState.depthFailOp = VK_STENCIL_OP_KEEP;
				stencilOpState.compareOp = VK_COMPARE_OP_ALWAYS;

				VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
				pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
				pipelineDepthStencilStateCreateInfo.depthTestEnable = pso.DepthTest;
				pipelineDepthStencilStateCreateInfo.depthWriteEnable = pso.DepthWrite;
				pipelineDepthStencilStateCreateInfo.depthCompareOp = CompareOpToVulkan(pso.DepthCompareOp);
				pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = false;
				pipelineDepthStencilStateCreateInfo.stencilTestEnable = false;
				pipelineDepthStencilStateCreateInfo.front = stencilOpState;
				pipelineDepthStencilStateCreateInfo.back = stencilOpState;

			
				VkColorComponentFlags colorComponentFlags = ColourComponentFlagsToVulkan(pso.ColourWriteMask);
				std::vector<VkPipelineColorBlendAttachmentState> pipeline_colour_blend_attachment_states;
				if (pso.Swapchain)
				{
					VkPipelineColorBlendAttachmentState blend_state = {};
					blend_state.blendEnable = pso.BlendEnable;
					blend_state.srcColorBlendFactor = BlendFactorToVulkan(pso.SrcColourBlendFactor);
					blend_state.dstColorBlendFactor = BlendFactorToVulkan(pso.DstColourBlendFactor);
					blend_state.colorBlendOp = BlendOpToVulkan(pso.ColourBlendOp);
					blend_state.srcAlphaBlendFactor = BlendFactorToVulkan(pso.SrcAplhaBlendFactor);
					blend_state.dstAlphaBlendFactor = BlendFactorToVulkan(pso.DstAplhaBlendFactor);
					blend_state.alphaBlendOp = BlendOpToVulkan(pso.AplhaBlendOp);
					blend_state.colorWriteMask = colorComponentFlags;

					pipeline_colour_blend_attachment_states.push_back(blend_state);
				}
				else
				{
					for (const RHI_Texture* tex : pso.RenderTargets)
					{
						if (tex)
						{
							VkPipelineColorBlendAttachmentState blend_state = {};
							blend_state.blendEnable = pso.BlendEnable;
							blend_state.srcColorBlendFactor = BlendFactorToVulkan(pso.SrcColourBlendFactor);
							blend_state.dstColorBlendFactor = BlendFactorToVulkan(pso.DstColourBlendFactor);
							blend_state.colorBlendOp = BlendOpToVulkan(pso.ColourBlendOp);
							blend_state.srcAlphaBlendFactor = BlendFactorToVulkan(pso.SrcAplhaBlendFactor);
							blend_state.dstAlphaBlendFactor = BlendFactorToVulkan(pso.DstAplhaBlendFactor);
							blend_state.alphaBlendOp = BlendOpToVulkan(pso.AplhaBlendOp);
							blend_state.colorWriteMask = colorComponentFlags;

							pipeline_colour_blend_attachment_states.push_back(blend_state);
						}
					}
				}

				VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
				pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				pipelineColorBlendStateCreateInfo.logicOpEnable = false;
				pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
				pipelineColorBlendStateCreateInfo.pAttachments = pipeline_colour_blend_attachment_states.data();
				pipelineColorBlendStateCreateInfo.attachmentCount = static_cast<u32>(pipeline_colour_blend_attachment_states.size());
				pipelineColorBlendStateCreateInfo.blendConstants[0] = 1.0f;
				pipelineColorBlendStateCreateInfo.blendConstants[1] = 1.0f;
				pipelineColorBlendStateCreateInfo.blendConstants[2] = 1.0f;
				pipelineColorBlendStateCreateInfo.blendConstants[3] = 1.0f;
			
				std::vector<VkDynamicState> dynamicStates = DynamicStatesToVulkan(pso.Dynamic_States);
				VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
				pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
				pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
				pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<u32>(dynamicStates.size());
			
				RenderContext* context = (RenderContext*)m_context;
				RHI_Renderpass rhiRenderpass = context->GetRenderpassManager().GetRenderpass(pso.Renderpass);
				VkRenderPass renderpass = reinterpret_cast<VkRenderPass>(rhiRenderpass.Resource);

				VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = { };
				graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();
				graphicsPipelineCreateInfo.stageCount = static_cast<u32>(pipelineShaderStageCreateInfos.size());
				graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo.CreateInfo;
				graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
				graphicsPipelineCreateInfo.pTessellationState = nullptr;
				graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
				graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
				graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
				graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
				graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
				graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
				graphicsPipelineCreateInfo.layout = layout;
				graphicsPipelineCreateInfo.renderPass = renderpass;

				if (pso.Swapchain)
				{
					pso.RenderTargets[0] = m_context->GetSwaphchainIamge();
				}

				std::vector<VkFormat> colourAttachmentFormats;
				VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
				VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
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

				VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {};
				pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
				pipelineRenderingCreateInfo.pColorAttachmentFormats = colourAttachmentFormats.data();
				pipelineRenderingCreateInfo.colorAttachmentCount = static_cast<u32>(colourAttachmentFormats.size());
				pipelineRenderingCreateInfo.depthAttachmentFormat = depthAttachmentFormat;
				pipelineRenderingCreateInfo.stencilAttachmentFormat = stencilAttachmentFormat;

				if (pso.AllowDynamicRendering 
					&& m_context->IsExtensionEnabled(DeviceExtension::VulkanDynamicRendering))
				{
					graphicsPipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;
					graphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE;
				}

				VkPipeline createdPipeline = nullptr;
				ThrowIfFailed(vkCreateGraphicsPipelines(m_context->GetDevice(), nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &createdPipeline));
				m_context->SetObjectName(pso.Name, (u64)createdPipeline, VK_OBJECT_TYPE_PIPELINE);
				m_pipelineStateObjects[psoHash] = createdPipeline;

				return createdPipeline;
			}

			void PipelineStateObjectManager_Vulkan::Destroy()
			{
				IS_PROFILE_FUNCTION();

				for (const auto pair : m_pipelineStateObjects)
				{
					vkDestroyPipeline(m_context->GetDevice(), pair.second, nullptr);
				}
				m_pipelineStateObjects.clear();
			}
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)