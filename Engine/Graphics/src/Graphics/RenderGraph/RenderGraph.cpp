#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderGraph/RenderGraphPass.h"
#include "Graphics/RenderContext.h"

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/RHI_CommandList.h"

#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/Window.h"

#include "Event/EventSystem.h"

#include "Core/Profiler.h"

#include <set>

namespace Insight
{
	namespace Graphics
	{
		RenderGraph::RenderGraph()
		{ }

		void RenderGraph::Init(RenderContext* context)
		{
			IS_PROFILE_FUNCTION();
			Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Graphics_Swapchain_Resize, [this](const Core::Event& event)
				{
					const Core::GraphcisSwapchainResize& resizeEvent = static_cast<const Core::GraphcisSwapchainResize&>(event);

					m_context->GpuWaitForIdle();
					//GetRenderPasses().clear();
					
					// Release all textures as the size they need to be has changed.
					m_textureCaches->Release();

					m_output_resolution = { resizeEvent.Width, resizeEvent.Height };
					if (m_set_render_resolution_to_window_resolution_auto)
					{
						SetRenderResolution(m_output_resolution);
					}	
				});

			m_context = context;
			for (size_t i = 0; i < m_context->GetFramesInFligtCount(); ++i)
			{
				m_pre_render_func.push_back(nullptr);
				m_post_render_func.push_back(nullptr);
				m_passes.push_back({});
			}

			m_textureCaches = Renderer::CreateTextureResourceCache();
		}

		void RenderGraph::Swap()
		{
			std::lock_guard lock(m_mutex);
			std::swap(m_passesUpdateIndex, m_passesRenderIndex);
			GetUpdatePasses().clear();
		}

		void RenderGraph::Execute(RHI_CommandList* cmdList)
		{
			IS_PROFILE_FUNCTION();
			ASSERT(m_context->IsRenderThread());

			/*
			* BUILD
			* Build each path. This should first create and make all textures on the GPU.
			* This could be done for one thread takes a pass to create all textures.
			* -
			* Build all shaders, this could be done in a similar fashion to textures.
			* Build all PSO.
			*
			* PLACE BARRIERS
			* Go through each pass and work out which textures/resources need barriers. This should then add to a barrier
			* list which should be done for each pass. Passes should have an incoming and outgoing list.
			*
			* EXECUTE ALL PASSES
			*/

			if (m_render_resolution_has_changed)
			{
				m_render_resolution_has_changed = false;

				m_context->GpuWaitForIdle();
				//GetRenderPasses().clear();

				// Release all current textures.
				m_textureCaches->Release();
			}

			Build();
			PlaceBarriers();

			RenderGraphSetPreRenderFunc preRenderFunc = m_pre_render_func.at(m_passesRenderIndex);
			if (preRenderFunc)
			{
				preRenderFunc(*this, cmdList);
			}

			Render(cmdList);

			RenderGraphSetPostRenderFunc postRenderFunc = m_post_render_func.at(m_passesRenderIndex);
			if (postRenderFunc)
			{
				postRenderFunc(*this, cmdList);
			}
		}

		RGTextureHandle RenderGraph::CreateTexture(std::string textureName, RHI_TextureInfo info)
		{
			ASSERT(m_context->IsRenderThread());
			return m_textureCaches->AddOrReturn(textureName);
		}

		RGTextureHandle RenderGraph::GetTexture(std::string textureName) const
		{
			ASSERT(m_context->IsRenderThread());
			return m_textureCaches->GetId(textureName);
		}

		RHI_Texture* RenderGraph::GetRHITexture(std::string textureName) const
		{
			ASSERT(m_context->IsRenderThread());
			return GetRHITexture(GetTexture(textureName));
		}

		RHI_Texture* RenderGraph::GetRHITexture(RGTextureHandle handle) const
		{
			ASSERT(m_context->IsRenderThread());
			return m_textureCaches->Get(handle);
		}

		RenderpassDescription RenderGraph::GetRenderpassDescription(std::string_view passName) const
		{
			ASSERT(m_context->IsRenderThread());
			auto itr = std::find_if(GetRenderPasses().begin(), GetRenderPasses().end(), [passName](const UPtr<RenderGraphPassBase>& pass)
				{
					return pass->m_passName == passName;
				});
			if (itr != GetRenderPasses().end())
			{
				return (*itr)->m_renderpassDescription;
			}
			return { };
		}

		PipelineStateObject RenderGraph::GetPipelineStateObject(std::string_view passName) const
		{
			ASSERT(m_context->IsRenderThread());
			auto itr = std::find_if(GetRenderPasses().begin(), GetRenderPasses().end(), [passName](const UPtr<RenderGraphPassBase>& pass)
				{
					return pass->m_passName == passName;
				});
			if (itr != GetRenderPasses().end())
			{
				return (*itr)->m_pso;
			}
			return { };
		}

		void RenderGraph::Release()
		{
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_mutex);

			m_context->GpuWaitForIdle();

			m_passes.clear();

			Renderer::FreeResourceCache(m_textureCaches);
		}

		void RenderGraph::Build()
		{
			IS_PROFILE_FUNCTION();
			ASSERT(m_context->IsRenderThread());

			RenderGraphBuilder builder(this);
			/// TODO This should be threaded. Leave as single thread for now.
			for (UPtr<RenderGraphPassBase>& pass : GetRenderPasses())
			{
				builder.SetPass(pass.Get());
				pass->Setup(builder);
				
				/// Build all our textures.
				for (auto& pair : pass.Get()->m_textureCreates)
				{
					RHI_Texture* tex = m_textureCaches->Get(pair.first);
					if (!tex->ValidResource())
					{
						pair.second.InitalStatus = DeviceUploadStatus::Completed;
						tex->Create(m_context, pair.second);
						tex->SetName(tex->m_name);
					}
				}

				PipelineStateObject& pso = pass.Get()->m_pso;
				pass->m_pso.Swapchain = pass->m_swapchainPass;
				pass->m_renderpassDescription.Pso = &pso;

				/// Build the shader here but no need to cache a reference to it as we 
				/// can lookup it up later. Just make sure it exists.
				pso.Shader = m_context->GetShaderManager().GetOrCreateShader(pass.Get()->m_shader);

				int rtIndex = 0;
				for (auto const& rt : pass.Get()->m_textureWrites)
				{
					if (rt != -1)
					{
						pso.RenderTargets[rtIndex] = m_textureCaches->Get(rt);
						++rtIndex;
						pass->m_renderpassDescription.ColourAttachments.push_back(m_textureCaches->Get(rt));
					}
				}
				pso.DepthStencil = m_textureCaches->Get(pass.Get()->m_depthStencilWrite);
				pass->m_renderpassDescription.DepthStencil = pso.DepthStencil;

				pass->m_renderpassDescription.SwapchainPass = pass->m_swapchainPass;

				m_context->GetRenderpassManager().GetOrCreateRenderpass(pass->m_renderpassDescription);
				pass->m_pso.Renderpass = pass->m_renderpassDescription.GetHash();
			}
		}

		void RenderGraph::PlaceBarriers()
		{
			IS_PROFILE_FUNCTION();
			ASSERT(m_context->IsRenderThread());

			struct FindImageBarrier
			{
				static ImageBarrier FindPrevious(const std::vector<UPtr<RenderGraphPassBase>>& passes, int currentPassIndex, RGTextureHandle handleToFind)
				{
					for (int i = currentPassIndex - 1; i >= 0; --i)
					{
						RenderGraphPassBase* pass = passes.at(i).Get();
						int textureIndex = 0;
						for (const auto& pipelineBarriers : pass->m_textureIncomingBarriers)
						{
							for (const auto& imageBarrier : pipelineBarriers.ImageBarriers)
							{
								if (imageBarrier.TextureHandle == handleToFind)
								{
									return imageBarrier;
								}
							}
						}
					}
					return { };
				}
			};

			struct PlaceInitalBarrier
			{
				static void PlaceBarrier(RHI_Texture* texture, std::vector<ImageBarrier>& previous_barriers)
				{
					/// Check if 
				}
			};

			int passIndex = 0;
			/// This should be threaded. Leave as single thread for now.
			for (UPtr<RenderGraphPassBase>& pass : GetRenderPasses())
			{
				PipelineBarrier colorPipelineBarrier;
				PipelineBarrier depthPipelineBarrier;

				std::vector<ImageBarrier> colorImageBarriers;
				std::vector<ImageBarrier> depthImageBarriers;

				/// Colour writes
				if (!pass->m_skipTextureWriteBarriers)
				{
					for (auto const& rt : pass.Get()->m_textureWrites)
					{
						RHI_Texture* texture = rt == -1 ? m_context->GetSwaphchainIamge() : m_textureCaches->Get(rt);
						
						PlaceInitalBarrier::PlaceBarrier(texture, m_texture_barrier_history[texture]);

						ImageBarrier previousBarrier = FindImageBarrier::FindPrevious(GetRenderPasses(), passIndex, rt);

						ImageBarrier barrier;
						barrier.TextureHandle = rt;
						barrier.Image = texture;

						barrier.SrcAccessFlags = AccessFlagBits::None;
						barrier.OldLayout = previousBarrier.IsValid() ? previousBarrier.NewLayout : texture->GetLayout();

						barrier.DstAccessFlags = AccessFlagBits::ColorAttachmentWrite;
						barrier.NewLayout = ImageLayout::ColourAttachment;
						barrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);

						m_texture_barrier_history[texture].push_back(barrier);
						colorImageBarriers.push_back(std::move(barrier));
					}
				}
				colorPipelineBarrier.SrcStage = +PipelineStageFlagBits::TopOfPipe;
				colorPipelineBarrier.DstStage = +PipelineStageFlagBits::ColourAttachmentOutput;
				colorPipelineBarrier.ImageBarriers = colorImageBarriers;

				/// Depth write
				if (pass->m_depthStencilWrite != -1)
				{
					RHI_Texture* texture = m_textureCaches->Get(pass->m_depthStencilWrite);
					PlaceInitalBarrier::PlaceBarrier(texture, m_texture_barrier_history[texture]);

					ImageBarrier previousBarrier = FindImageBarrier::FindPrevious(GetRenderPasses(), passIndex, pass->m_depthStencilWrite);

					ImageBarrier barrier;
					barrier.TextureHandle = pass->m_depthStencilWrite;
					barrier.Image = texture;

					barrier.SrcAccessFlags = AccessFlagBits::None;
					barrier.OldLayout = previousBarrier.IsValid() ? previousBarrier.NewLayout : texture->GetLayout();

					barrier.DstAccessFlags = AccessFlagBits::DepthStencilAttachmentWrite;
					barrier.NewLayout = ImageLayout::DepthStencilAttachment;
					barrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Depth);
					
					m_texture_barrier_history[texture].push_back(barrier);
					depthImageBarriers.push_back(std::move(barrier));
				}

				depthPipelineBarrier.SrcStage = +PipelineStageFlagBits::TopOfPipe;
				depthPipelineBarrier.DstStage = +PipelineStageFlagBits::EarlyFramgmentShader;
				depthPipelineBarrier.ImageBarriers = depthImageBarriers;

				if (colorPipelineBarrier.ImageBarriers.size() > 0 || colorPipelineBarrier.BufferBarriers.size() > 0)
				{
					pass->m_textureIncomingBarriers.push_back(colorPipelineBarrier);
				}
				if (depthPipelineBarrier.ImageBarriers.size() > 0 || depthPipelineBarrier.BufferBarriers.size() > 0)
				{
					pass->m_textureIncomingBarriers.push_back(depthPipelineBarrier);
				}

				colorPipelineBarrier = { };
				depthPipelineBarrier = { };
				colorImageBarriers = { };
				depthImageBarriers = { };

				/// Texture reads
				if (!pass->m_skipTextureReadBarriers)
				{
					for (auto const& rt : pass.Get()->m_textureReads)
					{
						RHI_Texture* texture = rt == -1 ? m_context->GetSwaphchainIamge() : m_textureCaches->Get(rt);
						PlaceInitalBarrier::PlaceBarrier(texture, m_texture_barrier_history[texture]);

						ImageBarrier previousBarrier = FindImageBarrier::FindPrevious(GetRenderPasses(), passIndex, rt);

						ImageBarrier barrier;
						barrier.Image = texture;
						barrier.TextureHandle = rt;
						bool isDepth = PixelFormatExtensions::IsDepth(barrier.Image->GetFormat());

						barrier.SrcAccessFlags = isDepth ? AccessFlagBits::DepthStencilAttachmentWrite : AccessFlagBits::ColorAttachmentWrite;
						barrier.OldLayout = previousBarrier.IsValid() ? previousBarrier.NewLayout : isDepth ?
							ImageLayout::DepthStencilAttachment : ImageLayout::ColourAttachment;

						barrier.DstAccessFlags = AccessFlagBits::ShaderRead;
						barrier.NewLayout = ImageLayout::ShaderReadOnly;
						barrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(isDepth ? ImageAspectFlagBits::Depth : ImageAspectFlagBits::Colour);

						m_texture_barrier_history[texture].push_back(barrier);
						if (isDepth)
						{
							depthImageBarriers.push_back(std::move(barrier));
						}
						else
						{
							colorImageBarriers.push_back(std::move(barrier));
						}
					}
				}
				colorPipelineBarrier.SrcStage = +PipelineStageFlagBits::ColourAttachmentOutput;
				colorPipelineBarrier.DstStage = +PipelineStageFlagBits::FragmentShader;
				colorPipelineBarrier.ImageBarriers = std::move(colorImageBarriers);

				depthPipelineBarrier.SrcStage = +PipelineStageFlagBits::EarlyFramgmentShader;
				depthPipelineBarrier.DstStage = +PipelineStageFlagBits::FragmentShader;
				depthPipelineBarrier.ImageBarriers = std::move(depthImageBarriers);
				
				if (colorPipelineBarrier.ImageBarriers.size() > 0 || colorPipelineBarrier.BufferBarriers.size() > 0)
				{
					pass->m_textureIncomingBarriers.push_back(colorPipelineBarrier);
				}
				if (depthPipelineBarrier.ImageBarriers.size() > 0 || depthPipelineBarrier.BufferBarriers.size() > 0)
				{
					pass->m_textureIncomingBarriers.push_back(depthPipelineBarrier);
				}

				++passIndex;
			}
		}

		void RenderGraph::Render(RHI_CommandList* cmdList)
		{
			IS_PROFILE_FUNCTION();
			ASSERT(m_context->IsRenderThread());

			/// TODO: Could be threaded? Leave as it is for now as it works.
			for (UPtr<RenderGraphPassBase>& pass : GetRenderPasses())
			{
				cmdList->BeginTimeBlock("PlaceBarriersInToPipeline", glm::vec4(1, 0, 0, 1));
				PlaceBarriersInToPipeline(pass.Get(), cmdList);
				cmdList->EndTimeBlock();

				cmdList->SetViewport(0.0f, 0.0f, (float)pass->m_viewport.x, (float)pass->m_viewport.y, 0.0f, 1.0f, false);
				cmdList->SetScissor(0, 0, pass->m_viewport.x, pass->m_viewport.y);

				std::string passName = std::string(pass->m_passName.begin(), pass->m_passName.end());
				///cmdList->BeginRenderpass(pass->m_renderpassDescription);
				cmdList->BeginTimeBlock(passName + "_Execute", glm::vec4(0, 1, 0, 1));
				pass->Execute(*this, cmdList);
				cmdList->EndTimeBlock();
				///cmdList->EndRenderpass();
			}

			for (UPtr<RenderGraphPassBase>& pass : GetRenderPasses())
			{
				pass->Post(*this, cmdList);
			}

			// If our swap chain image is not in the 'PresentSrc' layout then transition it.
			if (m_context->GetSwaphchainIamge()->GetLayout() != ImageLayout::PresentSrc)
			{
				PipelineBarrier barrier = { };
				barrier.SrcStage = +PipelineStageFlagBits::ColourAttachmentOutput;
				barrier.DstStage = +PipelineStageFlagBits::BottomOfPipe;

				ImageBarrier imageBarrier = { };
				imageBarrier.SrcAccessFlags = AccessFlagBits::ColorAttachmentWrite;
				imageBarrier.DstAccessFlags = AccessFlagBits::None;
				imageBarrier.Image = m_context->GetSwaphchainIamge();
				imageBarrier.OldLayout = ImageLayout::ColourAttachment;
				imageBarrier.NewLayout = ImageLayout::PresentSrc;
				imageBarrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);

				barrier.ImageBarriers.push_back(std::move(imageBarrier));
				cmdList->PipelineBarrier(barrier);
				//cmdList->SetImageLayout(m_context->GetSwaphchainIamge(), ImageLayout::PresentSrc);
			}
		}

		void RenderGraph::Clear()
		{
			IS_PROFILE_FUNCTION();
			m_passes.clear();
		}

		void RenderGraph::PlaceBarriersInToPipeline(RenderGraphPassBase* pass, RHI_CommandList* cmdList)
		{
			ASSERT(m_context->IsRenderThread());

			for (auto& barrier : pass->m_textureIncomingBarriers)
			{
				cmdList->PipelineBarrier(barrier);
			}
		}

		std::vector<UPtr<RenderGraphPassBase>>& RenderGraph::GetUpdatePasses()
		{
			return m_passes.at(m_passesUpdateIndex);
		}

		std::vector<UPtr<RenderGraphPassBase>>& RenderGraph::GetRenderPasses()
		{
			ASSERT(m_context->IsRenderThread());
			return m_passes.at(m_passesRenderIndex);
		}

		const std::vector<UPtr<RenderGraphPassBase>>& RenderGraph::GetUpdatePasses() const
		{
			return m_passes.at(m_passesUpdateIndex);
		}

		const std::vector<UPtr<RenderGraphPassBase>>& RenderGraph::GetRenderPasses() const
		{
			ASSERT(m_context->IsRenderThread());
			return m_passes.at(m_passesRenderIndex);
		}

		void RenderGraph::SetRenderResolution(glm::ivec2 render_resolution)
		{
			m_render_resolution = render_resolution;
			m_render_resolution_has_changed = true;
			Core::EventSystem::Instance().DispatchEvent(MakeRPtr<Core::GraphicsRenderResolutionChange>(m_render_resolution.x, m_render_resolution.y));
		}

		void RenderGraph::SetOutputResolution(glm::ivec2 output_resolution)
		{
			m_context->SetSwaphchainResolution(output_resolution);
			m_output_resolution = m_context->GetSwaphchainResolution();
		}
	}
}