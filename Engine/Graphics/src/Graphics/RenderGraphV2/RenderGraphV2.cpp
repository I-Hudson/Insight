#include "Graphics/RenderGraphV2/RenderGraphV2.h"
#include "Graphics/RenderGraphV2/RenderGraphPassV2.h"
#include "Graphics/RenderGraphV2/RenderGraphBuilderV2.h"
#include "Graphics/RenderContext.h"

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/RHI_CommandList.h"

#include "Graphics/PixelFormatExtensions.h"

#include "Event/EventSystem.h"

#include "Core/Profiler.h"

#include <nvtx3/nvtx3.hpp>

namespace Insight
{
	namespace Graphics
	{
		RenderGraphV2::RenderGraphV2()
		{ }

		void RenderGraphV2::Init(RenderContext* context)
		{
			IS_PROFILE_FUNCTION();
			Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Graphics_Swapchain_Resize, [this](const Core::Event& event)
				{
					const Core::GraphcisSwapchainResize& resizeEvent = static_cast<const Core::GraphcisSwapchainResize&>(event);

					m_output_resolution = { resizeEvent.Width, resizeEvent.Height };
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

		void RenderGraphV2::Swap()
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

		void RenderGraphV2::Execute(RHI_CommandList* cmdList)
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

				// Release all current textures.
				cmdList->BeginTimeBlock("RG::TextureCache->Release");
				m_textureCaches.ForEach([](RHI_ResourceCache<RHI_Texture>* textureCache)
				{
					textureCache->Release();
				});
				cmdList->EndTimeBlock();
			}

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

		RGTextureHandle RenderGraphV2::CreateTexture(std::string textureName, RHI_TextureInfo info)
		{
			ASSERT(m_context->IsRenderThread());
			return m_textureCaches.Get()->AddOrReturn(textureName);
		}

		RGTextureHandle RenderGraphV2::GetTexture(std::string textureName) const
		{
			ASSERT(m_context->IsRenderThread());
			return m_textureCaches.Get()->GetId(textureName);
		}

		RHI_Texture* RenderGraphV2::GetRHITexture(std::string textureName) const
		{
			ASSERT(m_context->IsRenderThread());
			return GetRHITexture(GetTexture(textureName));
		}

		RHI_Texture* RenderGraphV2::GetRHITexture(RGTextureHandle handle) const
		{
			ASSERT(m_context->IsRenderThread());
			return m_textureCaches.Get()->Get(handle);
		}

		RHI_Texture* RenderGraphV2::GetRenderCompletedRHITexture(std::string textureName) const
		{
			RGTextureHandle handle =  m_textureCaches.Get()->GetId(textureName);
			return m_textureCaches.Get()->Get(handle);
		}

		RenderpassDescription RenderGraphV2::GetRenderpassDescription(std::string_view passName) const
		{
			ASSERT(m_context->IsRenderThread());
			auto itr = std::find_if(GetRenderPasses().begin(), GetRenderPasses().end(), [passName](const UPtr<RenderGraphPassBaseV2>& pass)
				{
					return pass->m_passName == passName;
				});
			if (itr != GetRenderPasses().end())
			{
				return (*itr)->m_renderpassDescription;
			}
			return { };
		}

		PipelineStateObject RenderGraphV2::GetPipelineStateObject(std::string_view passName) const
		{
			ASSERT(m_context->IsRenderThread());
			auto itr = std::find_if(GetRenderPasses().begin(), GetRenderPasses().end(), [passName](const UPtr<RenderGraphPassBaseV2>& pass)
				{
					return pass->m_passName == passName;
				});
			if (itr != GetRenderPasses().end())
			{
				return (*itr)->m_PSO;
			}
			return { };
		}

		void RenderGraphV2::Release()
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

		void RenderGraphV2::AddSyncPoint(RenderGraphSyncFunc func)
		{
			m_syncFuncs.push_back(std::move(func));
		}

		void RenderGraphV2::AddPreRender(RenderGraphSetPreRenderFunc func)
		{
			std::lock_guard lock(m_mutex);
			m_preRenderFunc.push_back(std::move(func));
		}
		
		void RenderGraphV2::AddPostRender(RenderGraphSetPreRenderFunc func)
		{
			std::lock_guard lock(m_mutex);
			m_postRenderFunc.push_back(std::move(func));
		}

		void RenderGraphV2::Build()
		{
			IS_PROFILE_FUNCTION();
			ASSERT(m_context->IsRenderThread());

			RenderGraphBuilderV2 builder(this);
			//TODO Low: This should be threaded. Leave as single thread for now.
			for (UPtr<RenderGraphPassBaseV2>& pass : GetRenderPasses())
			{
				builder.SetPass(pass.Get());
				pass->Setup(RenderGraphSetupData { builder });
				
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

				PipelineStateObject& pso = pass.Get()->m_PSO;
				pass->m_PSO.Swapchain = pass->m_swapchainPass;
				pass->m_renderpassDescription.Pso = &pso;

				/// Build the shader here but no need to cache a reference to it as we 
				/// can lookup it up later. Just make sure it exists.
				pso.Shader = m_context->GetShaderManager().GetOrCreateShader(pass.Get()->m_shader);

				int rtIndex = 0;
				for (auto const& rt : pass.Get()->TextureWrites)
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
				pass->m_PSO.Renderpass = pass->m_renderpassDescription.GetHash();
			}
		}

		void RenderGraphV2::PlaceBarriers()
		{
			IS_PROFILE_FUNCTION();
			ASSERT(m_context->IsRenderThread());

			struct FindImageBarrier
			{
				static ImageBarrier FindPrevious(const std::vector<UPtr<RenderGraphPassBaseV2>>& passes, int currentPassIndex, RGTextureHandle handleToFind)
				{
					for (int i = currentPassIndex - 1; i >= 0; --i)
					{
						RenderGraphPassBaseV2* pass = passes.at(i).Get();
						int textureIndex = 0;
						for (const auto& pipelineBarriers : pass->m_incomingBarriers)
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
			for (UPtr<RenderGraphPassBaseV2>& pass : GetRenderPasses())
			{
				PipelineBarrier colorPipelineBarrier;
				PipelineBarrier depthPipelineBarrier;

				std::vector<ImageBarrier> colorImageBarriers;
				std::vector<ImageBarrier> depthImageBarriers;

				/// Colour writes
				//if (!pass->m_skipTextureWriteBarriers)
				{
					for (auto const& rt : pass.Get()->TextureWrites)
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
				colorPipelineBarrier.SrcStage = +PipelineStageFlagBits::TopOfPipe;
				colorPipelineBarrier.DstStage = +PipelineStageFlagBits::ColourAttachmentOutput;
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

				depthPipelineBarrier.SrcStage = +PipelineStageFlagBits::TopOfPipe;
				depthPipelineBarrier.DstStage = +PipelineStageFlagBits::EarlyFramgmentShader;
				depthPipelineBarrier.ImageBarriers = depthImageBarriers;

				if (colorPipelineBarrier.ImageBarriers.size() > 0 || colorPipelineBarrier.BufferBarriers.size() > 0)
				{
					pass->m_incomingBarriers.push_back(colorPipelineBarrier);
				}
				if (depthPipelineBarrier.ImageBarriers.size() > 0 || depthPipelineBarrier.BufferBarriers.size() > 0)
				{
					pass->m_incomingBarriers.push_back(depthPipelineBarrier);
				}

				colorPipelineBarrier = { };
				depthPipelineBarrier = { };
				colorImageBarriers = { };
				depthImageBarriers = { };

				/// Texture reads
				//if (!pass->m_skipTextureReadBarriers)
				{
					for (auto const& rt : pass.Get()->TextureReads)
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
				colorPipelineBarrier.SrcStage = +PipelineStageFlagBits::ColourAttachmentOutput;
				colorPipelineBarrier.DstStage = +PipelineStageFlagBits::FragmentShader;
				colorPipelineBarrier.ImageBarriers = std::move(colorImageBarriers);

				depthPipelineBarrier.SrcStage = +PipelineStageFlagBits::EarlyFramgmentShader;
				depthPipelineBarrier.DstStage = +PipelineStageFlagBits::FragmentShader;
				depthPipelineBarrier.ImageBarriers = std::move(depthImageBarriers);
				
				if (colorPipelineBarrier.ImageBarriers.size() > 0 || colorPipelineBarrier.BufferBarriers.size() > 0)
				{
					pass->m_incomingBarriers.push_back(colorPipelineBarrier);
				}
				if (depthPipelineBarrier.ImageBarriers.size() > 0 || depthPipelineBarrier.BufferBarriers.size() > 0)
				{
					pass->m_incomingBarriers.push_back(depthPipelineBarrier);
				}

				++passIndex;
			}
		}

		void RenderGraphV2::Render(RHI_CommandList* cmdList)
		{
			IS_PROFILE_FUNCTION();
			ASSERT(m_context->IsRenderThread());

			NVTX3_FUNC_RANGE();
			/// TODO Low: Could be threaded? Leave as it is for now as it works.
			for (UPtr<RenderGraphPassBaseV2>& pass : GetRenderPasses())
			{
				cmdList->BeginTimeBlock("PlaceBarriersInToPipeline", glm::vec4(1, 0, 0, 1));
				PlaceBarriersInToPipeline(pass.Get(), cmdList);
				cmdList->EndTimeBlock();

				cmdList->SetViewport(0.0f, 0.0f, (float)pass->m_viewport.x, (float)pass->m_viewport.y, 0.0f, 1.0f, false);
				cmdList->SetScissor(0, 0, pass->m_viewport.x, pass->m_viewport.y);

				std::string passName = std::string(pass->m_passName.begin(), pass->m_passName.end());
				cmdList->BeginTimeBlock(passName + "_Execute", glm::vec4(0, 1, 0, 1));
				GPUProfiler::Instance().StartProfile(cmdList, passName);
				pass->Execute(RenderGraphExecuteData { *this, cmdList});
				GPUProfiler::Instance().EndProfile(cmdList);
				cmdList->EndTimeBlock();
			}

			for (UPtr<RenderGraphPassBaseV2>& pass : GetRenderPasses())
			{
				pass->Post(RenderGraphPostData { *this, cmdList });
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
				cmdList->BeginTimeBlock("Transition swapchain image, common");
				cmdList->EndTimeBlock();
			}
		}

		void RenderGraphV2::Clear()
		{
			IS_PROFILE_FUNCTION();
			m_passes.clear();
		}

		void RenderGraphV2::PlaceBarriersInToPipeline(RenderGraphPassBaseV2* pass, RHI_CommandList* cmdList)
		{
			ASSERT(m_context->IsRenderThread());

			for (auto& barrier : pass->m_incomingBarriers)
			{
				cmdList->PipelineBarrier(barrier);
			}
		}

		std::vector<UPtr<RenderGraphPassBaseV2>>& RenderGraphV2::GetUpdatePasses()
		{
			return m_passes.at(m_passesUpdateIndex);
		}

		std::vector<UPtr<RenderGraphPassBaseV2>>& RenderGraphV2::GetRenderPasses()
		{
			ASSERT(m_context->IsRenderThread());
			return m_passes.at(m_passesRenderIndex);
		}

		const std::vector<UPtr<RenderGraphPassBaseV2>>& RenderGraphV2::GetUpdatePasses() const
		{
			return m_passes.at(m_passesUpdateIndex);
		}

		const std::vector<UPtr<RenderGraphPassBaseV2>>& RenderGraphV2::GetRenderPasses() const
		{
			ASSERT(m_context->IsRenderThread());
			return m_passes.at(m_passesRenderIndex);
		}

		void RenderGraphV2::SetRenderResolution(glm::ivec2 render_resolution)
		{
			m_render_resolution = render_resolution;
			m_render_resolution_has_changed = true;
			Core::EventSystem::Instance().DispatchEvent(MakeRPtr<Core::GraphicsRenderResolutionChange>(m_render_resolution.x, m_render_resolution.y));
		}

		void RenderGraphV2::SetOutputResolution(glm::ivec2 output_resolution)
		{
			m_context->SetSwaphchainResolution(output_resolution);
			m_output_resolution = m_context->GetSwaphchainResolution();
		}
	}
}