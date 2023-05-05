#include "Graphics/RenderContext.h"

#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"

#include "Graphics/RenderTarget.h"

#include "Core/Memory.h"
#include "Core/Logger.h"
#include "Core/EnginePaths.h"
#include "Core/Profiler.h"

#include "Algorithm/Vector.h"

#include "backends/imgui_impl_glfw.h"
#include <IconsFontAwesome5.h>

namespace Insight
{
	Graphics::RenderContext* Renderer::s_context;
	
#define ENABLE_IMGUI 1

	namespace Graphics
	{
		RenderContext::RenderContext()
			: m_renderTriggerSemaphore(0)
			, m_renderCompletedSemaphore(1)
		{ }

		RenderContext* RenderContext::New(GraphicsAPI graphicsAPI)
		{
			RenderContext* context = nullptr;
			switch (graphicsAPI)
			{
			case Insight::Graphics::GraphicsAPI::Vulkan:
#ifdef IS_VULKAN_ENABLED
				context = ::New<RHI::Vulkan::RenderContext_Vulkan, Insight::Core::MemoryAllocCategory::Graphics>();
#endif
				break;
			case Insight::Graphics::GraphicsAPI::DX12:
#ifdef IS_DX12_ENABLED
				context = ::New<RHI::DX12::RenderContext_DX12, Insight::Core::MemoryAllocCategory::Graphics>();
#endif
				break;
			default:
				FAIL_ASSERT();
				break;
			}

			if (!context)
			{
				IS_CORE_ERROR("[RenderContext* RenderContext::New] Unable to create a RenderContext.");
				return context;
			}

			::Insight::Renderer::s_context = context;
			context->m_graphicsAPI = graphicsAPI;
			context->m_samplerManager = RHI_SamplerManager::New();
			
			context->m_descriptorSetManager.Setup();
			context->m_commandListManager.Setup();

			context->m_shaderManager.SetRenderContext(context);
			context->m_descriptorLayoutManager.SetRenderContext(context);
			context->m_renderpassManager.SetRenderContext(context);
			context->m_samplerManager->SetRenderContext(context);

			context->m_renderGraph.Init(context);
			context->m_frameDescriptorAllocator.Setup();

			context->m_renderThreadId = std::this_thread::get_id();

			return context;
		}

		void RenderContext::Render()
		{
			if (m_desc.MultithreadContext)
			{
				{
					IS_PROFILE_SCOPE("Wait for Render Thread");
					m_renderCompletedSemaphore.Wait();
				}

				{
					IS_PROFILE_SCOPE("Swap");
					RenderStats::Instance().Draw();
					m_renderGraph.Swap();
				}

				{
					IS_PROFILE_SCOPE("Signal Render Thread");
					m_renderTriggerSemaphore.Signal();
				}
			}
			else
			{
				RenderStats::Instance().Draw();
				m_renderGraph.Swap();
				RenderUpdateLoop();
			}
		}

		bool RenderContext::IsRenderThread() const
		{
			return std::this_thread::get_id() == m_renderThreadId;
		}

		u32 RenderContext::GetFrameIndex() const
		{
			return m_frameIndex.load();
		}

		u64 RenderContext::GetFrameCount() const
		{
			return m_frameCount.load();
		}

		u32 RenderContext::GetFramesInFligtCount() const
		{
			return m_framesInFlightCount.load();
		}

		bool RenderContext::HasExtension(DeviceExtension extension) const
		{
			return m_deviceExtensions[(u32)extension] == 1;
		}

		bool RenderContext::IsExtensionEnabled(DeviceExtension extension) const
		{
			return m_enabledDeviceExtensions[(u32)extension] == 1;
		}

		void RenderContext::EnableExtension(DeviceExtension extension)
		{
			m_enabledDeviceExtensions[(u32)extension] = 1;
		}

		void RenderContext::DisableExtension(DeviceExtension extension)
		{
			m_enabledDeviceExtensions[(u32)extension] = 0;
		}

		bool RenderContext::IsRenderOptionsEnabled(RenderOptions option) const
		{
			return m_renderOptions.at(static_cast<u64>(option));
		}

		void RenderContext::EnabledRenderOption(RenderOptions option)
		{
			m_renderOptions.at(static_cast<u64>(option)) = true;
		}

		void RenderContext::DisableRenderOption(RenderOptions option)
		{
			m_renderOptions.at(static_cast<u64>(option)) = false;
		}

		void RenderContext::ImGuiBeginFrame()
		{
			if (m_font_texture == nullptr)
			{
				m_font_texture = Renderer::CreateTexture();
				m_font_texture->SetName("ImguiFontsTexture");

				unsigned char* pixels;
				int width, height;

				ImGuiIO& io = ImGui::GetIO();
				io.Fonts->AddFontDefault();

				// merge in icons from Font Awesome
				static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
				ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
				io.Fonts->AddFontFromFileTTF((std::string(EnginePaths::GetResourcePath() + "/fonts/") + FONT_ICON_FILE_NAME_FAS).c_str(), 16.0f, &icons_config, icons_ranges);
				// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

				io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
				m_font_texture->LoadFromData(pixels, width, height, 1, 4);

				ImTextureID texture_id = m_font_texture;
				io.Fonts->SetTexID(texture_id);
			}

			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
		}

		void RenderContext::ImGuiRender()
		{
			ImGui::Render();
			ImGui::UpdatePlatformWindows();
		}

		void RenderContext::ImGuiRelease()
		{
			if (m_font_texture)
			{
				Renderer::FreeTexture(m_font_texture);
				m_font_texture = nullptr;
			}
		}

		void RenderContext::BaseDestroy()
		{
			StopRenderThread();

			m_frameDescriptorAllocator.ForEach([](DescriptorAllocator& allocator)
				{
					allocator.Destroy();
				});

			m_renderGraph.Release();

			m_shaderManager.Destroy();
			m_renderpassManager.ReleaseAll();
			m_descriptorLayoutManager.ReleaseAll();
			m_descriptorSetManager.ForEach([](RHI_DescriptorSetManager& setManager)
				{
					setManager.ReleaseAll();
				});
			m_commandListManager.ForEach([](CommandListManager& manager)
				{
					manager.Destroy();
				});

			m_samplerManager->ReleaseAll();
			DeleteTracked(m_samplerManager);

			m_uploadQueue.Destroy();

			if (!m_resourceCaches.empty())
			{
				IS_CORE_WARN("[RenderContext::BaseDestroy] Not all RHI_ResourceCache's have been release with 'FreeResourceCache'. Please do this.");
				for (auto& cache : m_resourceCaches)
				{
					cache->Release();
					Delete(cache);
				}
				m_resourceCaches.clear();
			}

			if (!m_textures.IsEmpty())
			{
				IS_CORE_WARN("[RenderContext::BaseDestroy] Not all RHI_Texture's have been release with 'FreeTexture'. Please do this.");
				//m_textures.ReleaseAll();
			}

			for (auto& [bufferType, buffer] : m_buffers)
			{
				if (!buffer.IsEmpty())
				{
					IS_CORE_WARN("[RenderContext::BaseDestroy] Not all RHI_Buffer of type '{0}' have been release with the appropriate 'Free{0}Buffer'. Please do this.",
						BufferTypeToString[(int)bufferType]);
					buffer.ReleaseAll();
				}
			}
		}

		RHI_Buffer* RenderContext::CreateBuffer(BufferType bufferType, u64 sizeBytes, int stride, Graphics::RHI_Buffer_Overrides buffer_overrides)
		{
			RHI_Buffer* buffer = m_buffers[bufferType].CreateResource();
			buffer->Create(this, bufferType, sizeBytes, stride, buffer_overrides);
			buffer->SetName("Buffer");
			return buffer;
		}

		void RenderContext::FreeBuffer(RHI_Buffer* buffer)
		{
			if (buffer)
			{
				if (!m_resource_tracker.IsResourceInUse(buffer))
				{
					BufferType bufferType = buffer->GetType();
					m_buffers[bufferType].FreeResource(buffer);
				}
				else
				{
					BufferType bufferType = buffer->GetType();
					m_resource_tracker.AddDeferedRelase([this, buffer, bufferType]()
						{
							m_buffers[bufferType].FreeResource(buffer);
						});
				}
			}
		}

		int RenderContext::GetBufferCount(BufferType bufferType) const
		{
			const auto itr = m_buffers.find(bufferType);
			if (itr != m_buffers.end())
			{
				return itr->second.GetSize();
			}
			return 0;
		}

		RHI_Texture* RenderContext::CreateTextre()
		{
			RHI_Texture* texture =  m_textures.CreateResource();
			texture->SetName("Texture");
			return texture;
		}

		void RenderContext::FreeTexture(RHI_Texture* texture)
		{
			if (!m_resource_tracker.IsResourceInUse(texture))
			{
				m_textures.FreeResource(texture);
			}
			else
			{
				m_resource_tracker.AddDeferedRelase([this, texture]()
					{
						m_textures.FreeResource(texture);
					});
			}
		}

		void RenderContext::RenderUpdateLoop()
		{
			IS_PROFILE_FUNCTION();

			bool prepareRenderer = PrepareRender();

			RHI_CommandList* cmdList = nullptr;
			cmdList = GetCommandListManager().GetCommandList();
			if (prepareRenderer)
			{
				cmdList->m_descriptorAllocator = &m_frameDescriptorAllocator.Get();
				cmdList->m_descriptorAllocator->Reset();

				PreRender(cmdList);

				m_renderGraph.Execute(cmdList);

				if (cmdList->m_descriptorAllocator->WasUniformBufferResized())
				{
					cmdList->m_discard = true;
				}
			}
			else
			{
				ExecuteAsyncJobs(cmdList);
			}
			cmdList->Close();
			PostRender(cmdList);

			++m_frameCount;
		}

		void RenderContext::StartRenderThread()
		{
			m_renderThread = std::thread([this]()
			{
				IS_PROFILE_THREAD("Render Thread");
				while (!m_stopRenderThread)
				{
					IS_PROFILE_SCOPE("Render Update");
					{
						IS_PROFILE_SCOPE("Wait for Main Thread");
						m_renderTriggerSemaphore.Wait();
					}

					if (m_stopRenderThread)
					{
						break;
					}

					RenderUpdateLoop();
					{
						IS_PROFILE_SCOPE("Signal render complete");
						m_renderCompletedSemaphore.Signal();
					}
				}
			});
			m_renderThreadId = m_renderThread.get_id();
		}

		void RenderContext::StopRenderThread()
		{
			if (m_desc.MultithreadContext
				&& m_renderThreadId != std::thread::id())
			{
				m_stopRenderThread = true;
				m_renderTriggerSemaphore.Signal();
				m_renderThread.join();
				m_renderThreadId = m_renderThread.get_id();
			}
		}
	}

	/// Renderer

	Graphics::RHI_Buffer* Renderer::CreateVertexBuffer(u64 sizeBytes, int stride, Graphics::RHI_Buffer_Overrides buffer_overrides)
	{
		ASSERT(s_context);
		return s_context->CreateBuffer(Graphics::BufferType::Vertex, sizeBytes, stride, buffer_overrides);
	}

	Graphics::RHI_Buffer* Renderer::CreateIndexBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides)
	{
		ASSERT(s_context);
		return s_context->CreateBuffer(Graphics::BufferType::Index, sizeBytes, 0, buffer_overrides);
	}

	Graphics::RHI_Buffer* Renderer::CreateUniformBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides)
	{
		ASSERT(s_context);
		return s_context->CreateBuffer(Graphics::BufferType::Uniform, sizeBytes, 0, buffer_overrides);
	}

	Graphics::RHI_Buffer* Renderer::CreateRawBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides)
	{
		return s_context->CreateBuffer(Graphics::BufferType::Raw, sizeBytes, 0, buffer_overrides);
	}

	void Renderer::FreeVertexBuffer(Graphics::RHI_Buffer* buffer)
	{
		if (!buffer)
		{
			return;
		}
		ASSERT(s_context);
		s_context->FreeBuffer(buffer);
	}

	void Renderer::FreeIndexBuffer(Graphics::RHI_Buffer* buffer)
	{
		if (!buffer)
		{
			return;
		}
		ASSERT(s_context);
		ASSERT(buffer->GetType() == Graphics::BufferType::Index);
		s_context->FreeBuffer(buffer);
	}

	void Renderer::FreeUniformBuffer(Graphics::RHI_Buffer* buffer)
	{
		if (!buffer)
		{
			return;
		}
		ASSERT(s_context);
		ASSERT(buffer->GetType() == Graphics::BufferType::Uniform);
		s_context->FreeBuffer(buffer);
	}

	void Renderer::FreeRawBuffer(Graphics::RHI_Buffer* buffer)
	{
		if (!buffer)
		{
			return;
		}
		ASSERT(s_context);
		ASSERT(buffer->GetType() == Graphics::BufferType::Raw);
		s_context->FreeBuffer(buffer);
	}

	int Renderer::GetVertexBufferCount()
	{
		return s_context->GetBufferCount(Graphics::BufferType::Vertex);
	}

	int Renderer::GetIndexBufferCount()
	{
		return s_context->GetBufferCount(Graphics::BufferType::Index);
	}

	int Renderer::GetUniformBufferCount()
	{
		return s_context->GetBufferCount(Graphics::BufferType::Uniform);
	}

	int Renderer::GetBufferCount(Graphics::BufferType bufferType)
	{
		return s_context->GetBufferCount(bufferType);
	}

	Graphics::RHI_Texture* Renderer::CreateTexture()
	{
		return s_context->CreateTextre();
	}

	void Renderer::FreeTexture(Graphics::RHI_Texture* texture)
	{
		s_context->FreeTexture(texture);
	}

	Graphics::GraphicsAPI Renderer::GetGraphicsAPI()
	{
		return s_context->GetGraphicsAPI();
	}

	Graphics::RHI_ResourceCache<Graphics::RHI_Buffer>* Renderer::CreateBufferResourceCache(const Graphics::BufferType bufferType)
	{
		ASSERT(s_context);
		Graphics::RHI_ResourceCache<Graphics::RHI_Buffer>* resourceCache =
			New<Graphics::RHI_ResourceCache<Graphics::RHI_Buffer>, Core::MemoryAllocCategory::Graphics>(s_context->m_buffers.at(bufferType));
		s_context->m_resourceCaches.push_back(resourceCache);
		return resourceCache;
	}
	Graphics::RHI_ResourceCache<Graphics::RHI_Texture>* Renderer::CreateTextureResourceCache()
	{
		ASSERT(s_context);
		Graphics::RHI_ResourceCache<Graphics::RHI_Texture>* resourceCache = 
			New<Graphics::RHI_ResourceCache<Graphics::RHI_Texture>, Core::MemoryAllocCategory::Graphics>(s_context->m_textures);
		s_context->m_resourceCaches.push_back(resourceCache);
		return resourceCache;
	}

	void Renderer::FreeResourceCache(Graphics::IRHI_ResourceCache* resourceCache)
	{
		if (resourceCache == nullptr)
		{
			return;
		}
		ASSERT(s_context);
		Algorithm::VectorRemove(s_context->m_resourceCaches, resourceCache);
		resourceCache->Release();
		Delete(resourceCache);
	}
}