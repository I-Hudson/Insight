#ifdef RENDER_GRAPH_ENABLED

#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderGraph/RenderGraphPass.h"
#include "Graphics/RenderContext.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/PixelFormatExtensions.h"

#include "Event/EventManager.h"

#include <set>

namespace Insight
{
	namespace Graphics
	{
		u32 RenderGraph::s_FarmeCount = 2;

		RenderGraph::RenderGraph()
		{ }

		void RenderGraph::Init(RenderContext* context)
		{
			Core::EventManager::Instance().AddEventListener(this, Core::EventType::Graphics_Swapchain_Resize, [this](const Core::Event& event)
				{
					Release();
					m_commandListManager.ForEach([this](CommandListManager& manager)
						{
							manager.Create(m_context);
						});
				});

			m_context = context;

			m_textureCaches.Setup();
			m_commandListManager.Setup();

			m_commandListManager.Setup();
			m_commandListManager.ForEach([this](CommandListManager& manager)
			{
				manager.Create(m_context);
			});
		}

		void RenderGraph::Execute()
		{
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

			Build();
			PlaceBarriers();
			Render();
			Clear();

			m_frameIndex = (m_frameIndex + 1) % s_FarmeCount;
		}

		RGTextureHandle RenderGraph::CreateTexture(std::string textureName, RHI_TextureCreateInfo info)
		{
			return m_textureCaches->AddOrReturn(textureName);
		}

		RGTextureHandle RenderGraph::GetTexture(std::string textureName) const
		{
			return m_textureCaches->GetId(textureName);
		}

		void RenderGraph::Release()
		{
			m_context->GpuWaitForIdle();

			m_passes.clear();

			m_textureCaches.ForEach([](RHI_ResouceCache<RHI_Texture>& cache)
				{
					cache.Reset();
				});

			m_commandListManager.ForEach([](CommandListManager& manager)
				{
					manager.Destroy();
				});
		}

		void RenderGraph::Build()
		{
			RenderGraphBuilder builder(this);
			// TODO: This should be threaded. Leave as single thread for now.
			for (UPtr<RenderGraphPassBase>& pass : m_passes)
			{
				builder.SetPass(pass.Get());
				pass->Setup(builder);
				
				// Build all our textures.
				for (auto const& pair : pass.Get()->m_textureCreates)
				{
					RHI_Texture* tex = m_textureCaches->Get(pair.first);
					if (!tex->ValidResouce())
					{
						tex->Create(m_context, pair.second);
					}
				}

				// Build the shader here but no need to cache a reference to it as we 
				// can lookup it up later. Just make sure it exists.
				m_context->GetShaderManager().GetOrCreateShader(pass.Get()->m_shader);
			
				PipelineStateObject& pso = pass.Get()->m_pso;

				int rtIndex = 0;
				for (auto const& rt : pass.Get()->m_textureWrites)
				{
					pso.RenderTargets[rtIndex] = m_textureCaches->Get(rt);
					++rtIndex;
				}
				pso.DepthStencil = m_textureCaches->Get(pass.Get()->m_depthStencilWrite);

				m_context->GetRenderpassManager().GetOrCreateRenderpass(pass->m_renderpassDescription);
			}
		}

		void RenderGraph::PlaceBarriers()
		{
			int passIndex = 0;
			// This should be threaded. Leave as single thread for now.
			for (UPtr<RenderGraphPassBase>& pass : m_passes)
			{
				PipelineBarrier colorPipelineBarrier;
				PipelineBarrier depthPipelineBarrier;

				std::vector<ImageBarrier> colorImageBarriers;
				std::vector<ImageBarrier> depthImageBarriers;
				for (auto const& rt : pass.Get()->m_textureWrites)
				{
					ImageBarrier barrier;
					barrier.Image = m_textureCaches->Get(rt);
					bool isDepth = PixelFormatExtensions::IsDepth(barrier.Image->GetFormat());

					barrier.SrcAccessFlags = AccessFlagBits::None;
					barrier.OldLayout = ImageLayout::Undefined;

					barrier.DstAccessFlags = isDepth ? AccessFlagBits::DepthStencilAttachmentWrite  : AccessFlagBits::ColorAttachmentWrite;
					barrier.NewLayout = isDepth ? ImageLayout::DepthStencilAttachment : ImageLayout::ColourAttachment;
					barrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(isDepth ? ImageAspectFlagBits::Depth : ImageAspectFlagBits::Colour);

					if (isDepth)
					{
						depthImageBarriers.push_back(std::move(barrier));
					}
					else
					{
						colorImageBarriers.push_back(std::move(barrier));
					}
				}
				colorPipelineBarrier.SrcStage = PipelineStageFlagBits::TopOfPipe;
				colorPipelineBarrier.DstStage = PipelineStageFlagBits::ColourAttachmentOutput;
				colorPipelineBarrier.ImageBarriers = colorImageBarriers;

				depthPipelineBarrier.SrcStage = PipelineStageFlagBits::TopOfPipe;
				depthPipelineBarrier.DstStage = PipelineStageFlagBits::EarlyFramgmentShader;
				depthPipelineBarrier.ImageBarriers = depthImageBarriers;

				pass->m_textureIncomingBarriers.push_back(colorPipelineBarrier);
				pass->m_textureIncomingBarriers.push_back(depthPipelineBarrier);

				colorPipelineBarrier = { };
				depthPipelineBarrier = { };
				colorImageBarriers = { };
				depthImageBarriers = { };

				for (auto const& rt : pass.Get()->m_textureReads)
				{
					ImageBarrier barrier;
					barrier.Image = m_textureCaches->Get(rt);
					bool isDepth = PixelFormatExtensions::IsDepth(barrier.Image->GetFormat());

					barrier.SrcAccessFlags = AccessFlagBits::None;
					barrier.OldLayout = ImageLayout::Undefined;

					barrier.DstAccessFlags = AccessFlagBits::ShaderRead;
					barrier.NewLayout = ImageLayout::ShaderReadOnly;
					barrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(isDepth ? ImageAspectFlagBits::Depth : ImageAspectFlagBits::Colour);

					if (isDepth)
					{
						depthImageBarriers.push_back(std::move(barrier));
					}
					else
					{
						colorImageBarriers.push_back(std::move(barrier));
					}
				}
				colorPipelineBarrier.SrcStage = PipelineStageFlagBits::ColourAttachmentOutput;
				colorPipelineBarrier.DstStage = PipelineStageFlagBits::FragmentShader;
				colorPipelineBarrier.ImageBarriers = std::move(colorImageBarriers);

				depthPipelineBarrier.SrcStage = PipelineStageFlagBits::EarlyFramgmentShader;
				depthPipelineBarrier.DstStage = PipelineStageFlagBits::FragmentShader;
				depthPipelineBarrier.ImageBarriers = std::move(depthImageBarriers);

				pass->m_textureIncomingBarriers.push_back(colorPipelineBarrier);
				pass->m_textureIncomingBarriers.push_back(depthPipelineBarrier);

				++passIndex;
			}
		}

		void RenderGraph::Render()
		{
			if (m_context->PrepareRender())
			{
				RHI_CommandList* cmdList = m_commandListManager->GetCommandList();

				// TODO: Could be threaded? Leave as it is for now as it works.
				for (UPtr<RenderGraphPassBase>& pass : m_passes)
				{
					cmdList->SetViewport(0.0f, 0.0f, (float)pass->m_viewport.x, (float)pass->m_viewport.y, 0.0f, 1.0f);
					cmdList->SetScissor(0, 0, pass->m_viewport.x, pass->m_viewport.y);

					pass->m_pso.Shader = m_context->GetShaderManager().GetOrCreateShader(pass->m_shader);
					pass->m_pso.Swapchain = pass->m_swapchainPass;
					cmdList->SetPipeline(pass->m_pso);

					pass->Execute(cmdList);
				}

				cmdList->Close();

				m_context->PostRender(cmdList);
			}
		}

		void RenderGraph::Clear()
		{
			m_passes.clear();
		}
	}
}

#endif // RENDER_GRAPH_ENABLED