#include "Graphics/ImGuiPass.h"

#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsManager.h"

namespace Insight
{
	namespace Graphics
	{
		void ImGuiPass::Render()
		{
			if (m_vertex_buffer == nullptr)
			{
				m_vertex_buffer = Renderer::CreateVertexBuffer(sizeof(ImDrawVert), sizeof(ImDrawVert));
			}
			if (m_index_buffer == nullptr)
			{
				m_index_buffer = Renderer::CreateIndexBuffer(sizeof(ImDrawIdx));
			}
			if (m_font_texture == nullptr)
			{
				m_font_texture = Renderer::CreateTexture();

				unsigned char* pixels;
				int width, height;

				ImGuiIO& io = ImGui::GetIO();
				io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
				m_font_texture->LoadFromData(pixels, width, height, 1, 4);
				io.Fonts->SetTexID(m_font_texture);
			}

			struct TestPassData
			{
				PipelineStateObject Pso;
			};
			RenderGraph::Instance().AddPass<TestPassData>(L"ImGuiPass", [this](TestPassData& data, RenderGraphBuilder& builder)
				{
					builder.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetAsRenderToSwapchain();

					builder.WriteTexture(-1);

					ShaderDesc shaderDesc = { };
					shaderDesc.VertexFilePath = L"./Resources/Shaders/hlsl/ImGui.hlsl";
					shaderDesc.PixelFilePath = L"./Resources/Shaders/hlsl/ImGui.hlsl";
					shaderDesc.InputLayout =
					{
						ShaderInputLayout(0, PixelFormat::R32G32_Float, 0),
						ShaderInputLayout(1, PixelFormat::R32G32_Float, 8),
						ShaderInputLayout(2, PixelFormat::R8G8B8A8_UNorm, 16),
					};
					builder.SetShader(shaderDesc);

					PipelineStateObject pso = { };
					pso.Name = L"ImGui_PSO";
					pso.ShaderDescription = shaderDesc;

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

					builder.SetPipeline(pso);

					RenderpassDescription renderpassDescription = { };
					renderpassDescription.AddAttachment(AttachmentDescription::Load(PixelFormat::Unknown, Graphics::ImageLayout::PresentSrc));
					renderpassDescription.Attachments.back().InitalLayout = ImageLayout::ColourAttachment;
					builder.SetRenderpass(renderpassDescription);
				},
				[this](TestPassData& data, RenderGraph& renderGraph, RHI_CommandList* cmdList)
				{
					PipelineStateObject pso = renderGraph.GetPipelineStateObject(L"ImGuiPass");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(renderGraph.GetRenderpassDescription(L"ImGuiPass"));

					ImDrawData* draw_data = ImGui::GetDrawData();

					// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
					int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
					int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
					if (fb_width <= 0 || fb_height <= 0)
						return;

					size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
					size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
					if (vertex_size > m_vertex_buffer->GetSize())
					{
						m_vertex_buffer->Resize(vertex_size);
					}
					if (index_size > m_index_buffer->GetSize())
					{
						m_index_buffer->Resize(index_size);
					}

					int vtx_dst_offset = 0;
					int idx_dst_offset = 0;
					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = draw_data->CmdLists[n];
						const u64 vertex_buffer_size_bytes = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
						const u64 index_buffer_size_bytes = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);

						m_vertex_buffer->Upload(cmd_list->VtxBuffer.Data, vertex_buffer_size_bytes, vtx_dst_offset);
						m_index_buffer->Upload(cmd_list->IdxBuffer.Data, index_buffer_size_bytes, idx_dst_offset);
						vtx_dst_offset += vertex_buffer_size_bytes;
						idx_dst_offset += index_buffer_size_bytes;
					}

					cmdList->SetVertexBuffer(m_vertex_buffer);
					cmdList->SetIndexBuffer(m_index_buffer, IndexType::Uint16);
					cmdList->SetViewport(0, 0, Window::Instance().GetWidth(), Window::Instance().GetHeight(), 0.0f, 1.0f);

					// Setup scale and translation:
					// Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
					{
						float scale[2];
						scale[0] = 2.0f / draw_data->DisplaySize.x;
						scale[1] = 2.0f / draw_data->DisplaySize.y;
						float translate[2];
						translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
						translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
						cmdList->SetPushConstant(0, sizeof(scale), scale);
						cmdList->SetPushConstant(sizeof(scale), sizeof(translate), translate);
					}

					// Will project scissor/clipping rectangles into framebuffer space
					ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
					ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

					// Render command lists
					// (Because we merged all buffers into a single one, we maintain our own offset into them)
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
								// Project scissor/clipping rectangles into framebuffer space
								ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x)* clip_scale.x, (pcmd->ClipRect.y - clip_off.y)* clip_scale.y);
								ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x)* clip_scale.x, (pcmd->ClipRect.w - clip_off.y)* clip_scale.y);

								// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
								if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
								if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
								if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
								if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
								if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
								{
									continue;
								}

								// Apply scissor/clipping rectangle
								int scissor_offset_x = (int32_t)(clip_min.x);
								int scissor_offset_y = (int32_t)(clip_min.y);
								int scissor_extent_width = (uint32_t)(clip_max.x - clip_min.x);
								int scissor_extent_height = (uint32_t)(clip_max.y - clip_min.y);
								cmdList->SetScissor(scissor_offset_x, scissor_offset_y, scissor_extent_width, scissor_extent_height);

								cmdList->SetTexture(0, 0, static_cast<RHI_Texture*>(pcmd->TextureId));

								cmdList->DrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
							}
						}
						global_idx_offset += cmd_list->IdxBuffer.Size;
						global_vtx_offset += cmd_list->VtxBuffer.Size;
					}
					cmdList->EndRenderpass();
				});

		}

		void ImGuiPass::Release()
		{
			Renderer::FreeVertexBuffer(m_vertex_buffer);
			Renderer::FreeIndexBuffer(m_index_buffer);
			Renderer::FreeTexture(m_font_texture);
		}
	}
}