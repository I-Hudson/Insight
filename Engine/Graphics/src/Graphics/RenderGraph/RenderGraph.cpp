#ifdef RENDER_GRAPH_ENABLED

#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderGraph/RenderGraphPass.h"
#include "Graphics/RenderContext.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/Window.h"

#include "Event/EventManager.h"

#include "Core/Profiler.h"

#include <set>

namespace Insight
{
	namespace Graphics
	{
		u32 RenderGraph::s_MaxFarmeCount = 2;

		RenderGraph::RenderGraph()
		{ }

		void RenderGraph::Init(RenderContext* context)
		{
			IS_PROFILE_FUNCTION();
			Core::EventManager::Instance().AddEventListener(this, Core::EventType::Graphics_Swapchain_Resize, [this](const Core::Event& event)
				{
					Release();
					m_commandListManager.ForEach([this](CommandListManager& manager)
						{
							manager.Create(m_context);
						});
					m_textureCaches = NewTracked(RHI_ResourceCache<RHI_Texture>);
				});

			m_context = context;

			m_textureCaches = NewTracked(RHI_ResourceCache<RHI_Texture>);
			///m_textureCaches.Setup();
			m_commandListManager.Setup();

			m_commandListManager.Setup();
			m_commandListManager.ForEach([this](CommandListManager& manager)
			{
				manager.Create(m_context);
			});

			m_descriptorManagers.Setup();
			m_descriptorManagers.ForEach([this](DescriptorAllocator& alloc)
				{
					alloc.SetRenderContext(m_context);
				});
		}

		void RenderGraph::Execute()
		{
			IS_PROFILE_FUNCTION();
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

			if (m_context->PrepareRender())
			{
				Build();
				PlaceBarriers();
				m_commandListManager.Get().Reset();
				RHI_CommandList* cmdList = m_commandListManager.Get().GetCommandList();

				m_context->PreRender(cmdList);

				cmdList->m_descriptorAllocator = &m_descriptorManagers.Get();
				cmdList->m_descriptorAllocator->Reset();
				Render(cmdList);
				
				m_context->PostRender(cmdList);
				Clear();
				++m_frame_count;
			}

			m_frameIndex = (m_frameIndex + 1) % s_MaxFarmeCount;
		}

		RGTextureHandle RenderGraph::CreateTexture(std::wstring textureName, RHI_TextureCreateInfo info)
		{
			return m_textureCaches->AddOrReturn(textureName);
		}

		RGTextureHandle RenderGraph::GetTexture(std::wstring textureName) const
		{
			return m_textureCaches->GetId(textureName);
		}

		RHI_Texture* RenderGraph::GetRHITexture(RGTextureHandle handle) const
		{
			return m_textureCaches->Get(handle);
		}

		RenderpassDescription RenderGraph::GetRenderpassDescription(std::wstring_view passName) const
		{
			auto itr = std::find_if(m_passes.begin(), m_passes.end(), [passName](const UPtr<RenderGraphPassBase>& pass)
				{
					return pass->m_passName == passName;
				});
			if (itr != m_passes.end())
			{
				return (*itr)->m_renderpassDescription;
			}
			return { };
		}

		PipelineStateObject RenderGraph::GetPipelineStateObject(std::wstring_view passName) const
		{
			auto itr = std::find_if(m_passes.begin(), m_passes.end(), [passName](const UPtr<RenderGraphPassBase>& pass)
				{
					return pass->m_passName == passName;
				});
			if (itr != m_passes.end())
			{
				return (*itr)->m_pso;
			}
			return { };
		}

		void RenderGraph::Release()
		{
			IS_PROFILE_FUNCTION();
			m_context->GpuWaitForIdle();

			m_passes.clear();

			m_textureCaches->Reset();
			DeleteTracked(m_textureCaches);
			///m_textureCaches.ForEach([](RHI_ResourceCache<RHI_Texture>& cache)
			///	{
			///		cache.Reset();
			///	});

			m_commandListManager.ForEach([](CommandListManager& manager)
				{
					manager.Destroy();
				});

			m_descriptorManagers.ForEach([](DescriptorAllocator& allocator)
				{
					allocator.Destroy();
				});
		}

		void RenderGraph::Build()
		{
			IS_PROFILE_FUNCTION();
			RenderGraphBuilder builder(this);
			/// TODO This should be threaded. Leave as single thread for now.
			for (UPtr<RenderGraphPassBase>& pass : m_passes)
			{
				builder.SetPass(pass.Get());
				pass->Setup(builder);
				
				/// Build all our textures.
				for (auto const& pair : pass.Get()->m_textureCreates)
				{
					RHI_Texture* tex = m_textureCaches->Get(pair.first);
					if (!tex->ValidResouce())
					{
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
			for (UPtr<RenderGraphPassBase>& pass : m_passes)
			{
				PipelineBarrier colorPipelineBarrier;
				PipelineBarrier depthPipelineBarrier;

				std::vector<ImageBarrier> colorImageBarriers;
				std::vector<ImageBarrier> depthImageBarriers;

				/// Colour writes
				for (auto const& rt : pass.Get()->m_textureWrites)
				{
					RHI_Texture* texture = rt == -1 ? m_context->GetSwaphchainIamge() : m_textureCaches->Get(rt);
					PlaceInitalBarrier::PlaceBarrier(texture, m_texture_barrier_history[texture]);

					ImageBarrier previousBarrier = FindImageBarrier::FindPrevious(m_passes, passIndex, rt);

					ImageBarrier barrier;
					barrier.TextureHandle = rt;
					barrier.Image = texture;

					barrier.SrcAccessFlags = AccessFlagBits::None;
					barrier.OldLayout = previousBarrier.IsValid() ? previousBarrier.NewLayout : ImageLayout::Undefined;

					barrier.DstAccessFlags = AccessFlagBits::ColorAttachmentWrite;
					barrier.NewLayout = ImageLayout::ColourAttachment;
					barrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer( ImageAspectFlagBits::Colour);
					
					m_texture_barrier_history[texture].push_back(barrier);
					colorImageBarriers.push_back(std::move(barrier));
				}
				colorPipelineBarrier.SrcStage = PipelineStageFlagBits::TopOfPipe;
				colorPipelineBarrier.DstStage = PipelineStageFlagBits::ColourAttachmentOutput;
				colorPipelineBarrier.ImageBarriers = colorImageBarriers;

				/// Depth write
				if (pass->m_depthStencilWrite != -1)
				{
					RHI_Texture* texture = m_textureCaches->Get(pass->m_depthStencilWrite);
					PlaceInitalBarrier::PlaceBarrier(texture, m_texture_barrier_history[texture]);

					ImageBarrier previousBarrier = FindImageBarrier::FindPrevious(m_passes, passIndex, pass->m_depthStencilWrite);

					ImageBarrier barrier;
					barrier.TextureHandle = pass->m_depthStencilWrite;
					barrier.Image = texture;

					barrier.SrcAccessFlags = AccessFlagBits::None;
					barrier.OldLayout = previousBarrier.IsValid() ? previousBarrier.NewLayout : ImageLayout::Undefined;

					barrier.DstAccessFlags = AccessFlagBits::DepthStencilAttachmentWrite;
					barrier.NewLayout = ImageLayout::DepthStencilAttachment;
					barrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Depth);
					
					m_texture_barrier_history[texture].push_back(barrier);
					depthImageBarriers.push_back(std::move(barrier));
				}

				depthPipelineBarrier.SrcStage = PipelineStageFlagBits::TopOfPipe;
				depthPipelineBarrier.DstStage = PipelineStageFlagBits::EarlyFramgmentShader;
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
				for (auto const& rt : pass.Get()->m_textureReads)
				{
					RHI_Texture* texture = rt == -1 ? m_context->GetSwaphchainIamge() : m_textureCaches->Get(rt);
					PlaceInitalBarrier::PlaceBarrier(texture, m_texture_barrier_history[texture]);

					ImageBarrier previousBarrier = FindImageBarrier::FindPrevious(m_passes, passIndex, rt);

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
				colorPipelineBarrier.SrcStage = PipelineStageFlagBits::ColourAttachmentOutput;
				colorPipelineBarrier.DstStage = PipelineStageFlagBits::FragmentShader;
				colorPipelineBarrier.ImageBarriers = std::move(colorImageBarriers);

				depthPipelineBarrier.SrcStage = PipelineStageFlagBits::EarlyFramgmentShader;
				depthPipelineBarrier.DstStage = PipelineStageFlagBits::FragmentShader;
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

			/// TODO: Could be threaded? Leave as it is for now as it works.
			for (UPtr<RenderGraphPassBase>& pass : m_passes)
			{
				PlaceBarriersInToPipeline(pass.Get(), cmdList);

				cmdList->SetViewport(0.0f, 0.0f, (float)pass->m_viewport.x, (float)pass->m_viewport.y, 0.0f, 1.0f, false);
				cmdList->SetScissor(0, 0, pass->m_viewport.x, pass->m_viewport.y);


				///cmdList->BeginRenderpass(pass->m_renderpassDescription);
				pass->Execute(*this, cmdList);
				///cmdList->EndRenderpass();
			}

			if (m_context->IsExtensionEnabled(DeviceExtension::VulkanDynamicRendering))
			{
				PipelineBarrier barrier = { };
				barrier.SrcStage = PipelineStageFlagBits::ColourAttachmentOutput;
				barrier.DstStage = PipelineStageFlagBits::BottomOfPipe;

				ImageBarrier imageBarrier = { };
				imageBarrier.SrcAccessFlags = AccessFlagBits::ColorAttachmentWrite;
				imageBarrier.DstAccessFlags = AccessFlagBits::None;
				imageBarrier.Image = m_context->GetSwaphchainIamge();
				imageBarrier.OldLayout = ImageLayout::ColourAttachment;
				imageBarrier.NewLayout = ImageLayout::PresentSrc;
				imageBarrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);

				barrier.ImageBarriers.push_back(std::move(imageBarrier));
				cmdList->PipelineBarrier(barrier);
			}

			cmdList->Close();
		}

		void RenderGraph::Clear()
		{
			IS_PROFILE_FUNCTION();
			m_passes.clear();
		}

		void RenderGraph::PlaceBarriersInToPipeline(RenderGraphPassBase* pass, RHI_CommandList* cmdList)
		{
			for (auto& barrier : pass->m_textureIncomingBarriers)
			{
				cmdList->PipelineBarrier(barrier);
			}
		}
	}
}

#endif /// RENDER_GRAPH_ENABLED