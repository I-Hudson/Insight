#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderGraph/RenderGraphPass.h"
#include "Graphics/RenderGraph/RenderGraphBuilder.h"
#include "Graphics/RenderContext.h"

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/RHI_CommandList.h"

#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/Window.h"

#include "Event/EventSystem.h"

#include "Core/Profiler.h"
#include "Core/Logger.h"

#include <nvtx3/nvtx3.hpp>

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

					m_output_resolution = Maths::Vector2(resizeEvent.Width, resizeEvent.Height);
					if (m_set_render_resolution_to_window_resolution_auto)
					{
						SetRenderResolution(m_output_resolution);
					}	
				});

			m_context = context;
			for (size_t i = 0; i < m_context->GetFramesInFligtCount(); ++i)
			{
				m_passes.push_back({});
			}

			m_textureCaches.Setup();
			m_textureCaches.ForEach([](RHI_ResourceCache<RHI_Texture>*& textureCache)
				{
					textureCache = Renderer::CreateTextureResourceCache();
				}); 
		}

		void RenderGraph::Swap()
		{
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_mutex);
			std::swap(m_passesUpdateIndex, m_passesRenderIndex);
			GetUpdatePasses().clear();

			for (RenderGraphSyncFunc& func : m_syncFuncs)
			{
				func();
			}
			m_syncFuncs.clear();

			m_renderPreRenderFunc = m_preRenderFunc;
			m_renderPostRenderFunc = m_postRenderFunc;
			m_preRenderFunc.clear();
			m_postRenderFunc.clear();
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
				m_render_resolution_has_changedSkip2ndFrame = true;

				m_context->GpuWaitForIdle();

				// Release all current textures.
				cmdList->BeginTimeBlock("RG::TextureCache->Release");
				m_textureCaches.ForEach([](RHI_ResourceCache<RHI_Texture>* textureCache)
				{
					textureCache->Release();
				});
				cmdList->EndTimeBlock();


			}
			else if (m_render_resolution_has_changedSkip2ndFrame)
			{
				m_render_resolution_has_changedSkip2ndFrame = false;
			}
			else
			{
				cmdList->BeginTimeBlock("RG::PreRenderFunc");
				Build();
				cmdList->EndTimeBlock();

				cmdList->BeginTimeBlock("RG::PlaceBarriers");
				PlaceBarriers();
				cmdList->EndTimeBlock();

				for (RenderGraphSetPreRenderFunc& preRenderFunc : m_renderPreRenderFunc)
				{
					cmdList->BeginTimeBlock("RG::PreRenderFunc");
					if (preRenderFunc)
					{
						preRenderFunc(*this, cmdList);
					}
					cmdList->EndTimeBlock();
				}

				Render(cmdList);

				for (RenderGraphSetPostRenderFunc& postRenderFunc : m_renderPostRenderFunc)
				{
					cmdList->BeginTimeBlock("RG::PostRenderFunc");
					if (postRenderFunc)
					{
						postRenderFunc(*this, cmdList);
					}
					cmdList->EndTimeBlock();
				}
				cmdList->EndTimeBlock();
			}
		}

		RGTextureHandle RenderGraph::CreateTexture(std::string textureName, RHI_TextureInfo info)
		{
			ASSERT(m_context->IsRenderThread());
			return m_textureCaches.Get()->AddOrReturn(textureName);
		}

		RGTextureHandle RenderGraph::GetTexture(std::string textureName) const
		{
			ASSERT(m_context->IsRenderThread());
			return m_textureCaches.Get()->GetId(textureName);
		}

		RHI_Texture* RenderGraph::GetRHITexture(std::string textureName) const
		{
			ASSERT(m_context->IsRenderThread());
			return GetRHITexture(GetTexture(textureName));
		}

		RHI_Texture* RenderGraph::GetRHITexture(RGTextureHandle handle) const
		{
			ASSERT(m_context->IsRenderThread());
			return m_textureCaches.Get()->Get(handle);
		}

		RHI_Texture* RenderGraph::GetRenderCompletedRHITexture(std::string textureName) const
		{
			RGTextureHandle handle =  m_textureCaches.Get()->GetId(textureName);
			return m_textureCaches.Get()->Get(handle);
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

		ComputePipelineStateObject RenderGraph::GetComputePipelineStateObject(std::string_view psoName) const
		{
			ASSERT(m_context->IsRenderThread());
			auto itr = std::find_if(GetRenderPasses().begin(), GetRenderPasses().end(), [psoName](const UPtr<RenderGraphPassBase>& pass)
				{
					return pass->m_computePSO.Name == psoName;
				});
			if (itr != GetRenderPasses().end())
			{
				return (*itr)->m_computePSO;
			}
			return { };
		}

		void RenderGraph::Release()
		{
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_mutex);

			m_context->GpuWaitForIdle();

			m_passes.clear();

			m_textureCaches.ForEach([](RHI_ResourceCache<RHI_Texture>*& textureCache)
			{
				Renderer::FreeResourceCache(textureCache);
				textureCache = nullptr;
			});
		}

		void RenderGraph::AddSyncPoint(RenderGraphSyncFunc func)
		{
			m_syncFuncs.push_back(std::move(func));
		}

		void RenderGraph::AddPreRender(RenderGraphSetPreRenderFunc func)
		{
			std::lock_guard lock(m_mutex);
			m_preRenderFunc.push_back(std::move(func));
		}
		
		void RenderGraph::AddPostRender(RenderGraphSetPreRenderFunc func)
		{
			std::lock_guard lock(m_mutex);
			m_postRenderFunc.push_back(std::move(func));
		}

		void RenderGraph::Build()
		{
			IS_PROFILE_FUNCTION();
			ASSERT(m_context->IsRenderThread());

			RenderGraphBuilder builder(this);
			//TODO Low: This should be threaded. Leave as single thread for now.
			for (UPtr<RenderGraphPassBase>& pass : GetRenderPasses())
			{
				builder.SetPass(pass.Get());
				pass->Setup(builder);
				
				/// Build all our textures.
				for (auto& pair : pass.Get()->m_textureCreates)
				{
					RHI_Texture* tex = m_textureCaches.Get()->Get(pair.first);
					if (!tex->ValidResource())
					{
						pair.second.InitalStatus = DeviceUploadStatus::Completed;
						tex->SetName(tex->m_name);
						tex->Create(m_context, pair.second);
					}
				}

				PipelineStateObject& pso = pass.Get()->m_pso;
				pass->m_pso.Swapchain = pass->m_swapchainPass;
				pass->m_renderpassDescription.Pso = &pso;

				/// Build the shader here but no need to cache a reference to it as we 
				/// can lookup it up later. Just make sure it exists.
				pso.Shader = m_context->GetShaderManager().GetOrCreateShader(pass.Get()->m_shader);

				ComputePipelineStateObject& computePSO = pass.Get()->m_computePSO;
				computePSO.Shader = m_context->GetShaderManager().GetOrCreateShader(computePSO.ShaderDescription);

				int rtIndex = 0;
				for (auto const& rt : pass.Get()->m_textureWrites)
				{
					if (rt != -1)
					{
						pso.RenderTargets[rtIndex] = m_textureCaches.Get()->Get(rt);
						++rtIndex;
						pass->m_renderpassDescription.ColourAttachments.push_back(m_textureCaches.Get()->Get(rt));
					}
				}
				pso.DepthStencil = m_textureCaches.Get()->Get(pass.Get()->m_depthStencilWrite);
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

			std::unordered_map<RHI_Texture*, std::vector<ImageBarrier>> texture_barrier_history;
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
						RHI_Texture* texture = rt == -1 ? m_context->GetSwaphchainIamge() : m_textureCaches.Get()->Get(rt);
						

						PlaceInitalBarrier::PlaceBarrier(texture, texture_barrier_history[texture]);

						ImageBarrier previousBarrier = FindImageBarrier::FindPrevious(GetRenderPasses(), passIndex, rt);

						ImageBarrier barrier;
						barrier.TextureHandle = rt;
						barrier.Image = texture;

						barrier.SrcAccessFlags = AccessFlagBits::None;
						barrier.OldLayout = previousBarrier.IsValid() ? previousBarrier.NewLayout : texture->GetLayout();

						barrier.DstAccessFlags = AccessFlagBits::ColorAttachmentWrite;
						barrier.NewLayout = ImageLayout::ColourAttachment;
						barrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);

						texture_barrier_history[texture].push_back(barrier);
						colorImageBarriers.push_back(std::move(barrier));
					}
				}
				colorPipelineBarrier.SrcStage = static_cast<u32>(PipelineStageFlagBits::TopOfPipe);
				colorPipelineBarrier.DstStage = static_cast<u32>(PipelineStageFlagBits::ColourAttachmentOutput);
				colorPipelineBarrier.ImageBarriers = colorImageBarriers;

				/// Depth write
				if (pass->m_depthStencilWrite != -1)
				{
					RHI_Texture* texture = m_textureCaches.Get()->Get(pass->m_depthStencilWrite);
					PlaceInitalBarrier::PlaceBarrier(texture, texture_barrier_history[texture]);

					ImageBarrier previousBarrier = FindImageBarrier::FindPrevious(GetRenderPasses(), passIndex, pass->m_depthStencilWrite);

					ImageBarrier barrier;
					barrier.TextureHandle = pass->m_depthStencilWrite;
					barrier.Image = texture;

					barrier.SrcAccessFlags = AccessFlagBits::None;
					barrier.OldLayout = previousBarrier.IsValid() ? previousBarrier.NewLayout : texture->GetLayout();

					barrier.DstAccessFlags = AccessFlagBits::DepthStencilAttachmentWrite;
					barrier.NewLayout = ImageLayout::DepthStencilAttachment;
					barrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Depth);
					
					texture_barrier_history[texture].push_back(barrier);
					depthImageBarriers.push_back(std::move(barrier));
				}

				depthPipelineBarrier.SrcStage = static_cast<u32>(PipelineStageFlagBits::TopOfPipe);
				depthPipelineBarrier.DstStage = static_cast<u32>(PipelineStageFlagBits::EarlyFramgmentShader);
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
						RHI_Texture* texture = rt == -1 ? m_context->GetSwaphchainIamge() : m_textureCaches.Get()->Get(rt);
						PlaceInitalBarrier::PlaceBarrier(texture, texture_barrier_history[texture]);

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

						texture_barrier_history[texture].push_back(barrier);
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
				colorPipelineBarrier.SrcStage = static_cast<u32>(PipelineStageFlagBits::ColourAttachmentOutput);
				colorPipelineBarrier.DstStage = static_cast<u32>(PipelineStageFlagBits::FragmentShader);
				colorPipelineBarrier.ImageBarriers = std::move(colorImageBarriers);

				depthPipelineBarrier.SrcStage = static_cast<u32>(PipelineStageFlagBits::EarlyFramgmentShader);
				depthPipelineBarrier.DstStage = static_cast<u32>(PipelineStageFlagBits::FragmentShader);
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

			NVTX3_FUNC_RANGE();
			/// TODO Low: Could be threaded? Leave as it is for now as it works.
			for (UPtr<RenderGraphPassBase>& pass : GetRenderPasses())
			{
				cmdList->BeginTimeBlock("PlaceBarriersInToPipeline", Maths::Vector4(1, 0, 0, 1));
				PlaceBarriersInToPipeline(pass.Get(), cmdList);
				cmdList->EndTimeBlock();

				cmdList->SetViewport(0.0f, 0.0f, pass->m_viewport.x, pass->m_viewport.y, 0.0f, 1.0f, false);
				cmdList->SetScissor(0, 0, (int)pass->m_viewport.x, (int)pass->m_viewport.y);

				std::string passName = std::string(pass->m_passName.begin(), pass->m_passName.end());
				cmdList->BeginTimeBlock(passName + "_Execute", Maths::Vector4(0, 1, 0, 1));
				GPUProfiler::Instance().StartProfile(cmdList, passName);
				pass->Execute(*this, cmdList);
				GPUProfiler::Instance().EndProfile(cmdList);
				cmdList->EndTimeBlock();
			}

			for (UPtr<RenderGraphPassBase>& pass : GetRenderPasses())
			{
				pass->Post(*this, cmdList);
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

		void RenderGraph::SetRenderResolution(Maths::Vector2 render_resolution)
		{
			if (m_render_resolution != render_resolution)
			{
				m_render_resolution = render_resolution;
				m_render_resolution_has_changed = true;
				Core::EventSystem::Instance().DispatchEvent(MakeRPtr<Core::GraphicsRenderResolutionChange>(m_render_resolution.x, m_render_resolution.y));
			}
		}

		void RenderGraph::SetOutputResolution(Maths::Vector2 output_resolution)
		{
			if (m_output_resolution != output_resolution)
			{
				m_context->SetSwaphchainResolution(output_resolution);
				m_output_resolution = m_context->GetSwaphchainResolution();
			}
		}
	}
}