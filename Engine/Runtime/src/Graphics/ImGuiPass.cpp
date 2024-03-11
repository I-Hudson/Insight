#include "Graphics/ImGuiPass.h"

#include "Graphics/RenderGraph/RenderGraph.h"

#ifdef RENDERGRAPH_V2_ENABLED
#include "Graphics/RenderGraphV2/RenderGraphV2.h"
#include "Graphics/RenderGraphV2/RenderGraphPassV2.h"
#endif

#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"

#include "Core/Profiler.h"
#include "Core/EnginePaths.h"

#include "Asset/AssetRegistry.h"

namespace Insight
{
	namespace Graphics
	{
		ImGuiPass::ImGuiPassData::~ImGuiPassData()
		{
			ASSERT(ImGui::GetCurrentContext());
		}

		void ImGuiPass::ImGuiPassData::Reset()
		{
			for (ImDrawList& drawList : CmdList)
			{
				drawList._ResetForNewFrame();
			}
		}

		void ImGuiPass::Create()
		{
			RHI_Buffer_Overrides buffer_overrides = {};
			buffer_overrides.Force_Host_Writeable = true;
			if (m_vertex_buffer.Size() == 0)
			{
				m_vertex_buffer.Setup();
				m_vertex_buffer.ForEach([buffer_overrides](RHI_Buffer*& buffer)
					{
						buffer = Renderer::CreateVertexBuffer(sizeof(ImDrawVert), sizeof(ImDrawVert), buffer_overrides);
					});
			}
			if (m_index_buffer.Size() == 0)
			{
				m_index_buffer.Setup();
				m_index_buffer.ForEach([buffer_overrides](RHI_Buffer*& buffer)
					{
						buffer = Renderer::CreateIndexBuffer(sizeof(ImDrawIdx), buffer_overrides);
					});
			}

			std::vector<Byte> shaderData = Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/Shaders/hlsl/ImGui.hlsl");
			ShaderDesc shaderDesc("ImGui", shaderData, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
			shaderDesc.InputLayout =
			{
				ShaderInputLayout(0, PixelFormat::R32G32_Float, offsetof(ImDrawVert, pos), "POSITION"),
				ShaderInputLayout(1, PixelFormat::R32G32_Float, offsetof(ImDrawVert, uv), "TEXCOORD0"),
				ShaderInputLayout(2, PixelFormat::R8G8B8A8_UNorm, offsetof(ImDrawVert, col), "COLOR0"),
			};
			RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);
		}

		void ImGuiPass::Render()
		{
			RenderContext::Instance().ImGuiRender();

			ImGuiPassData& passData = RemoveConst(m_passData.GetCurrent());
			passData.Reset();
			passData.ImDrawData = std::move(*ImGui::GetDrawData());

			for (size_t i = 0; i < passData.ImDrawData.CmdListsCount; ++i)
			{
				ImDrawList* imguiDrawList = passData.ImDrawData.CmdLists[i];
				ImDrawList* drawList = nullptr;
				if (i < passData.CmdList.size())
				{
					drawList = &passData.CmdList.at(i);
				}
				else
				{
					passData.CmdList.push_back((ImGui::GetDrawListSharedData()));
					drawList = &passData.CmdList.back();

					passData.CmdListPtrs.resize(passData.CmdList.size());
					for (size_t drawListIdx = 0; drawListIdx < passData.CmdList.size(); ++drawListIdx)
					{
						passData.CmdListPtrs[drawListIdx] = &passData.CmdList[drawListIdx];
					}
				}

				{
					IS_PROFILE_SCOPE("Set pass imgui draw data");
					drawList->CmdBuffer = imguiDrawList->CmdBuffer;
					drawList->VtxBuffer = imguiDrawList->VtxBuffer;
					drawList->IdxBuffer = imguiDrawList->IdxBuffer;
				}
			}
			ImDrawList** drawListPtr = passData.CmdListPtrs.data();
			passData.ImDrawData.CmdLists = drawListPtr;

			//RenderContext::Instance().ImGuiBeginFrame();

			struct ImguiPass
			{
				ImDrawData* ImDrawData;
			};

#ifdef RENDERGRAPH_V2_ENABLED
			ImGuiPassData imGuiRenderData = m_passData.GetCurrent();
			RenderGraphV2::Instance().AddGraphicsPass("ImGuiPass"
				, [](RenderGraphGraphicsPassV2& pass)
				{
					IS_PROFILE_SCOPE("ImGui pass setup");

					pass.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					pass.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					pass.SetAsRenderToSwapchain();

					pass.WriteTexture(-1);

					ShaderDesc shaderDesc("ImGui", { }, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
					shaderDesc.InputLayout =
					{
						ShaderInputLayout(0, PixelFormat::R32G32_Float, offsetof(ImDrawVert, pos), "POSITION"),
						ShaderInputLayout(1, PixelFormat::R32G32_Float, offsetof(ImDrawVert, uv), "TEXCOORD0"),
						ShaderInputLayout(2, PixelFormat::R8G8B8A8_UNorm, offsetof(ImDrawVert, col), "COLOR0"),
					};
					pass.SetShader(shaderDesc);

					PipelineStateObject pso = { };
					pso.Name = "ImGui_PSO";
					pso.ShaderDescription = shaderDesc;
					pso.DepthWrite = false;
					pso.DepthTest = false;

					pso.PolygonMode = PolygonMode::Fill;
					pso.CullMode = CullMode::None;
					pso.FrontFace = FrontFace::CounterClockwise;

					pso.BlendEnable = true;
					pso.SrcColourBlendFactor = BlendFactor::SrcAlpha;
					pso.DstColourBlendFactor = BlendFactor::OneMinusSrcAlpha;
					pso.ColourBlendOp = BlendOp::Add;
					pso.SrcAplhaBlendFactor = BlendFactor::One;
					pso.DstAplhaBlendFactor = BlendFactor::OneMinusSrcAlpha;
					pso.AplhaBlendOp = BlendOp::Add;
					pso.AllowDynamicRendering = false;

					pass.SetPipeline(pso);

					RenderpassDescription renderpassDescription = { };
					renderpassDescription.AddAttachment(AttachmentDescription::Load(PixelFormat::Unknown, Graphics::ImageLayout::PresentSrc));
					renderpassDescription.Attachments.back().InitalLayout = Graphics::ImageLayout::ColourAttachment;
					renderpassDescription.AllowDynamicRendering = false;

					pass.SetRenderpass(renderpassDescription);
				},
				[&, imGuiRenderData](const RenderGraphExecuteData& executeData)
				{
					IS_PROFILE_SCOPE("ImGui pass execute");

					RenderGraphV2& renderGraph = executeData.RenderGraph;
					RHI_CommandList* cmdList = executeData.CmdList;

					PipelineStateObject pso = renderGraph.GetPipelineStateObject("ImGuiPass");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(renderGraph.GetRenderpassDescription("ImGuiPass"));

					RHI_SamplerCreateInfo bilinearSamplerInfo = { };
					bilinearSamplerInfo.MagFilter = Filter::Linear;
					bilinearSamplerInfo.MinFilter = Filter::Linear;
					bilinearSamplerInfo.MipmapMode = SamplerMipmapMode::Linear;
					bilinearSamplerInfo.AddressMode = SamplerAddressMode::Repeat;
					bilinearSamplerInfo.CompareEnabled = false;
					RHI_Sampler* bilinearSampler = RenderContext::Instance().GetSamplerManager().GetOrCreateSampler(bilinearSamplerInfo);

					cmdList->SetSampler(2, 0, bilinearSampler);

					ImDrawData* imguiDrawData = ImGui::GetDrawData();
					const ImDrawData* draw_data = &imGuiRenderData.ImDrawData;

					/// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
					int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
					int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
					if (fb_width <= 0 || fb_height <= 0)
						return;

					size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
					size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
					if (vertex_size > m_vertex_buffer.Get()->GetSize())
					{
						m_vertex_buffer.Get()->Resize(vertex_size + (sizeof(ImDrawVert) * 1000));
					}
					if (index_size > m_index_buffer.Get()->GetSize())
					{
						m_index_buffer.Get()->Resize(index_size + (sizeof(ImDrawIdx) * 1000));
					}

					u64 vtx_dst_offset = 0;
					u64 idx_dst_offset = 0;
					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = draw_data->CmdLists[n];
						const u64 vertex_buffer_size_bytes = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
						const u64 index_buffer_size_bytes = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);

						m_vertex_buffer.Get()->Upload(cmd_list->VtxBuffer.Data, vertex_buffer_size_bytes, vtx_dst_offset, 0);
						m_index_buffer.Get()->Upload(cmd_list->IdxBuffer.Data, index_buffer_size_bytes, idx_dst_offset, 0);
						vtx_dst_offset += vertex_buffer_size_bytes;
						idx_dst_offset += index_buffer_size_bytes;
					}

					cmdList->SetVertexBuffer(m_vertex_buffer.Get());
					cmdList->SetIndexBuffer(m_index_buffer.Get(), IndexType::Uint16);
					cmdList->SetViewport(0, 0
						, static_cast<float>(Window::Instance().GetWidth()), static_cast<float>(Window::Instance().GetHeight())
						, 0.0f, 1.0f);

					/// Setup scale and translation:
					/// Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
					{
						if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
						{
							struct UBO
							{
								float Scale[2];
								float Translate[2];
							};
							UBO ubo = {};
							ubo.Scale[0] = 2.0f / draw_data->DisplaySize.x;
							ubo.Scale[1] = 2.0f / draw_data->DisplaySize.y;
							ubo.Translate[0] = -1.0f - draw_data->DisplayPos.x * ubo.Scale[0];
							ubo.Translate[1] = -1.0f - draw_data->DisplayPos.y * ubo.Scale[1];

							cmdList->SetUniform(0, 0, ubo);
						}
						else if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12)
						{
							struct VERTEX_CONSTANT_BUFFER_DX12
							{
								float   mvp[4][4];
							};

							// Setup orthographic projection matrix into our constant buffer
							// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right).
							VERTEX_CONSTANT_BUFFER_DX12 vertex_constant_buffer;
							{
								float L = draw_data->DisplayPos.x;
								float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
								float T = draw_data->DisplayPos.y;
								float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
								float mvp[4][4] =
								{
									{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
									{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
									{ 0.0f,         0.0f,           0.5f,       0.0f },
									{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
								};
								memcpy(&vertex_constant_buffer.mvp, mvp, sizeof(mvp));
							}
							RHI_BufferView view = cmdList->UploadUniform(vertex_constant_buffer);
							cmdList->SetUniform(0, 0, view);
						}
					}

					/// Will project scissor/clipping rectangles into framebuffer space
					ImVec2 clip_off = draw_data->DisplayPos;         /// (0,0) unless using multi-viewports
					ImVec2 clip_scale = draw_data->FramebufferScale; /// (1,1) unless using retina display which are often (2,2)

					/// Render command lists
					/// (Because we merged all buffers into a single one, we maintain our own offset into them)
					int global_vtx_offset = 0;
					int global_idx_offset = 0;
					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = draw_data->CmdLists[n];
						for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
						{
							const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
							if (pcmd->UserCallback != NULL)
							{
								pcmd->UserCallback(cmd_list, pcmd);
							}
							else
							{

								/// Project scissor/clipping rectangles into framebuffer space
								ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
								ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

								/// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
								if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
								if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
								if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
								if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
								if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
								{
									continue;
								}

								/// Apply scissor/clipping rectangle
								int scissor_offset_x = (int32_t)(clip_min.x);
								int scissor_offset_y = (int32_t)(clip_min.y);
								int scissor_extent_width = (uint32_t)(clip_max.x - clip_min.x);
								int scissor_extent_height = (uint32_t)(clip_max.y - clip_min.y);
								if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
								{
									cmdList->SetScissor(scissor_offset_x, scissor_offset_y, scissor_extent_width, scissor_extent_height);
								}
								else if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12)
								{
									/// Project scissor/clipping rectangles into framebuffer space
									ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
									ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
									if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
										continue;

									cmdList->SetScissor((int)clip_min.x, (int)clip_min.y, (int)clip_max.x, (int)clip_max.y);
								}
								RHI_Texture* texture = static_cast<RHI_Texture*>(pcmd->TextureId);

								if (texture)
								{
									// TODO High: HACK REALLY BAD. There needs to be a system where you can predefine multiple pipelines before rendering.
									// This would allow for at runtime pipelines to already be created.
									PipelineStateObject pso = renderGraph.GetPipelineStateObject("ImGuiPass");
									if (texture->m_name == "ImguiFontsTexture")
									{
										cmdList->BindPipeline(pso, false);
									}
									else
									{
										pso.Name = "ImGuiPass_NoBlend";
										pso.BlendEnable = false;
										cmdList->BindPipeline(pso, false);
									}
									PipelineBarrier pipelineBarrier;
									pipelineBarrier.SrcStage = (u32)PipelineStageFlagBits::ColourAttachmentOutput;
									pipelineBarrier.DstStage = (u32)PipelineStageFlagBits::FragmentShader;

									ImageBarrier imageBarrier;
									imageBarrier.SrcAccessFlags = AccessFlagBits::ColorAttachmentWrite;
									imageBarrier.DstAccessFlags = AccessFlagBits::ShaderRead;
									imageBarrier.OldLayout = texture->GetLayout();
									imageBarrier.NewLayout = ImageLayout::ShaderReadOnly;
									imageBarrier.Image = texture;
									imageBarrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);
									pipelineBarrier.ImageBarriers.push_back(imageBarrier);

									cmdList->PipelineBarrier(pipelineBarrier);
									cmdList->SetTexture(1, 0, texture);
								}

								cmdList->DrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
							}
						}
						global_idx_offset += cmd_list->IdxBuffer.Size;
						global_vtx_offset += cmd_list->VtxBuffer.Size;
					}
					cmdList->EndRenderpass();
				},
				{});
#endif

			RenderGraph::Instance().AddPass<ImguiPass>("ImGuiPass", [this](ImguiPass& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("ImGui pass setup");

					builder.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetAsRenderToSwapchain();

					builder.WriteTexture(-1);

					ShaderDesc shaderDesc("ImGui", { }, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
					shaderDesc.InputLayout =
					{
						ShaderInputLayout(0, PixelFormat::R32G32_Float, offsetof(ImDrawVert, pos), "POSITION"),
						ShaderInputLayout(1, PixelFormat::R32G32_Float, offsetof(ImDrawVert, uv), "TEXCOORD0"),
						ShaderInputLayout(2, PixelFormat::R8G8B8A8_UNorm, offsetof(ImDrawVert, col), "COLOR0"),
					};
					builder.SetShader(shaderDesc);

					PipelineStateObject pso = { };
					pso.Name = "ImGui_PSO";
					pso.ShaderDescription = shaderDesc;
					pso.DepthWrite = false;
					pso.DepthTest = false;

					pso.PolygonMode = PolygonMode::Fill;
					pso.CullMode = CullMode::None;
					pso.FrontFace = FrontFace::CounterClockwise;

					pso.BlendEnable = true;
					pso.SrcColourBlendFactor = BlendFactor::SrcAlpha;
					pso.DstColourBlendFactor = BlendFactor::OneMinusSrcAlpha;
					pso.ColourBlendOp = BlendOp::Add;
					pso.SrcAplhaBlendFactor = BlendFactor::One;
					pso.DstAplhaBlendFactor = BlendFactor::OneMinusSrcAlpha;
					pso.AplhaBlendOp = BlendOp::Add;
					pso.AllowDynamicRendering = false;

					builder.SetPipeline(pso);

					RenderpassDescription renderpassDescription = { };
					renderpassDescription.AddAttachment(AttachmentDescription::Load(PixelFormat::Unknown, Graphics::ImageLayout::PresentSrc));
					renderpassDescription.Attachments.back().InitalLayout = Graphics::ImageLayout::ColourAttachment;
					renderpassDescription.AllowDynamicRendering = false;

					builder.SetRenderpass(renderpassDescription);
				},
				[&](ImguiPass& data, RenderGraph& renderGraph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("ImGui pass execute");

					PipelineStateObject pso = renderGraph.GetPipelineStateObject("ImGuiPass");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(renderGraph.GetRenderpassDescription("ImGuiPass"));

					RHI_SamplerCreateInfo bilinearSamplerInfo = { };
					bilinearSamplerInfo.MagFilter = Filter::Linear;
					bilinearSamplerInfo.MinFilter = Filter::Linear;
					bilinearSamplerInfo.MipmapMode = SamplerMipmapMode::Linear;
					bilinearSamplerInfo.AddressMode = SamplerAddressMode::Repeat;
					bilinearSamplerInfo.CompareEnabled = false;
					RHI_Sampler* bilinearSampler = RenderContext::Instance().GetSamplerManager().GetOrCreateSampler(bilinearSamplerInfo);

					cmdList->SetSampler(2, 0, bilinearSampler);

					ImDrawData* imguiDrawData = ImGui::GetDrawData();
					ImDrawData* draw_data = data.ImDrawData;

					/// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
					int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
					int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
					if (fb_width <= 0 || fb_height <= 0)
						return;

					size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
					size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
					if (vertex_size > m_vertex_buffer.Get()->GetSize())
					{
						m_vertex_buffer.Get()->Resize(vertex_size + (sizeof(ImDrawVert) * 1000));
					}
					if (index_size > m_index_buffer.Get()->GetSize())
					{
						m_index_buffer.Get()->Resize(index_size + (sizeof(ImDrawIdx) * 1000));
					}

					u64 vtx_dst_offset = 0;
					u64 idx_dst_offset = 0;
					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = draw_data->CmdLists[n];
						const u64 vertex_buffer_size_bytes = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
						const u64 index_buffer_size_bytes = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);

						m_vertex_buffer.Get()->Upload(cmd_list->VtxBuffer.Data, vertex_buffer_size_bytes, vtx_dst_offset, 0);
						m_index_buffer.Get()->Upload(cmd_list->IdxBuffer.Data, index_buffer_size_bytes, idx_dst_offset, 0);
						vtx_dst_offset += vertex_buffer_size_bytes;
						idx_dst_offset += index_buffer_size_bytes;
					}

					cmdList->SetVertexBuffer(m_vertex_buffer.Get());
					cmdList->SetIndexBuffer(m_index_buffer.Get(), IndexType::Uint16);
					cmdList->SetViewport(0, 0
						, static_cast<float>(Window::Instance().GetWidth()), static_cast<float>(Window::Instance().GetHeight())
						, 0.0f, 1.0f);

					/// Setup scale and translation:
					/// Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
					{
						if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
						{
							struct UBO
							{
								float Scale[2];
								float Translate[2];
							};
							UBO ubo = {};
							ubo.Scale[0] = 2.0f / draw_data->DisplaySize.x;
							ubo.Scale[1] = 2.0f / draw_data->DisplaySize.y;
							ubo.Translate[0] = -1.0f - draw_data->DisplayPos.x * ubo.Scale[0];
							ubo.Translate[1] = -1.0f - draw_data->DisplayPos.y * ubo.Scale[1];

							cmdList->SetUniform(0, 0, ubo);
						}
						else if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12)
						{
							struct VERTEX_CONSTANT_BUFFER_DX12
							{
								float   mvp[4][4];
							};

							// Setup orthographic projection matrix into our constant buffer
							// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right).
							VERTEX_CONSTANT_BUFFER_DX12 vertex_constant_buffer;
							{
								float L = draw_data->DisplayPos.x;
								float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
								float T = draw_data->DisplayPos.y;
								float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
								float mvp[4][4] =
								{
									{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
									{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
									{ 0.0f,         0.0f,           0.5f,       0.0f },
									{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
								};
								memcpy(&vertex_constant_buffer.mvp, mvp, sizeof(mvp));
							}
							RHI_BufferView view = cmdList->UploadUniform(vertex_constant_buffer);
							cmdList->SetUniform(0, 0, view);
						}
					}

					/// Will project scissor/clipping rectangles into framebuffer space
					ImVec2 clip_off = draw_data->DisplayPos;         /// (0,0) unless using multi-viewports
					ImVec2 clip_scale = draw_data->FramebufferScale; /// (1,1) unless using retina display which are often (2,2)

					/// Render command lists
					/// (Because we merged all buffers into a single one, we maintain our own offset into them)
					int global_vtx_offset = 0;
					int global_idx_offset = 0;
					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = draw_data->CmdLists[n];
						for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
						{
							const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
							if (pcmd->UserCallback != NULL)
							{
								pcmd->UserCallback(cmd_list, pcmd);
							}
							else
							{

								/// Project scissor/clipping rectangles into framebuffer space
								ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
								ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

								/// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
								if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
								if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
								if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
								if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
								if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
								{
									continue;
								}

								/// Apply scissor/clipping rectangle
								int scissor_offset_x = (int32_t)(clip_min.x);
								int scissor_offset_y = (int32_t)(clip_min.y);
								int scissor_extent_width = (uint32_t)(clip_max.x - clip_min.x);
								int scissor_extent_height = (uint32_t)(clip_max.y - clip_min.y);
								if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
								{
									cmdList->SetScissor(scissor_offset_x, scissor_offset_y, scissor_extent_width, scissor_extent_height);
								}
								else if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12)
								{
									/// Project scissor/clipping rectangles into framebuffer space
									ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
									ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
									if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
										continue;

									cmdList->SetScissor((int)clip_min.x, (int)clip_min.y, (int)clip_max.x, (int)clip_max.y);
								}
								RHI_Texture* texture = static_cast<RHI_Texture*>(pcmd->TextureId);
								//ASSERT(RenderGraph::Instance().HasTexture(texture) || Renderer::HasTexture(texture));

								if (texture)
								{
									// TODO High: HACK REALLY BAD. There needs to be a system where you can predefine multiple pipelines before rendering.
									// This would allow for at runtime pipelines to already be created.
									PipelineStateObject pso = renderGraph.GetPipelineStateObject("ImGuiPass");
									if (texture->m_name == "ImguiFontsTexture"
										|| texture->HasAplha())
									{
										cmdList->BindPipeline(pso, false);
									}
									else
									{
										pso.Name = "ImGuiPass_NoBlend";
										pso.BlendEnable = false;
										cmdList->BindPipeline(pso, false);
									}
									PipelineBarrier pipelineBarrier;
									pipelineBarrier.SrcStage = (u32)PipelineStageFlagBits::ColourAttachmentOutput;
									pipelineBarrier.DstStage = (u32)PipelineStageFlagBits::FragmentShader;

									ImageBarrier imageBarrier;
									imageBarrier.SrcAccessFlags = AccessFlagBits::ColorAttachmentWrite;
									imageBarrier.DstAccessFlags = AccessFlagBits::ShaderRead;
									imageBarrier.OldLayout = texture->GetLayout();
									imageBarrier.NewLayout = ImageLayout::ShaderReadOnly;
									imageBarrier.Image = texture;
									imageBarrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);
									pipelineBarrier.ImageBarriers.push_back(imageBarrier);

									cmdList->PipelineBarrier(pipelineBarrier);
									cmdList->SetTexture(1, 0, texture);
								}

								cmdList->DrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
							}
						}
						global_idx_offset += cmd_list->IdxBuffer.Size;
						global_vtx_offset += cmd_list->VtxBuffer.Size;
					}
					cmdList->EndRenderpass();
				}
			, std::move(ImguiPass{ &passData.ImDrawData }));

			m_passData.Swap();
		}

		void ImGuiPass::Release()
		{
			m_vertex_buffer.ForEach([](RHI_Buffer*& buffer)
				{
					Renderer::FreeVertexBuffer(buffer);
					buffer = nullptr;
				});
			m_index_buffer.ForEach([](RHI_Buffer*& buffer)
				{
					Renderer::FreeVertexBuffer(buffer);
					buffer = nullptr;
				});
		}
	}
}