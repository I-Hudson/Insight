#include "Graphics/RenderContext.h"


#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"

#include "Graphics/RenderTarget.h"

#include "Core/Memory.h"
#include "Core/Logger.h"

#include "backends/imgui_impl_glfw.h"

namespace Insight
{
	Graphics::RenderContext* Renderer::s_context;
	
#define ENABLE_IMGUI 1

	namespace Graphics
	{
		RenderContext* RenderContext::New(GraphicsAPI graphicsAPI)
		{
			RenderContext* context = nullptr;
			switch (graphicsAPI)
			{
			case Insight::Graphics::GraphicsAPI::Vulkan:
#ifdef IS_VULKAN_ENABLED
				context = NewTracked(RHI::Vulkan::RenderContext_Vulkan);
#endif
				break;
			case Insight::Graphics::GraphicsAPI::DX12:
#ifdef IS_DX12_ENABLED
				context = NewTracked(RHI::DX12::RenderContext_DX12);
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

			return context;
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

		void RenderContext::ImGuiBeginFrame()
		{
			if (m_font_texture == nullptr)
			{
				m_font_texture = Renderer::CreateTexture();

				unsigned char* pixels;
				int width, height;

				ImGuiIO& io = ImGui::GetIO();
				io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
				m_font_texture->LoadFromData(pixels, width, height, 1, 4);

				ImTextureID texture_id = m_font_texture;
				io.Fonts->SetTexID(texture_id);
			}

			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

			RenderStats::Instance().Draw();
		}

		void RenderContext::ImGuiRender()
		{
			ImGui::Render();
			ImGui::UpdatePlatformWindows();
		}

		void RenderContext::BaseDestroy()
		{
			m_descriptorLayoutManager.ReleaseAll();
			m_textures.ReleaseAll();
			m_shaderManager.Destroy();
			m_renderpassManager.ReleaseAll();
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

			if (m_font_texture)
			{
				Renderer::FreeTexture(m_font_texture);
				m_font_texture = nullptr;
			}

			m_uploadQueue.Destroy();

			for (auto& buffer : m_buffers)
			{
				buffer.second.ReleaseAll();
			}
		}

		RHI_Buffer* RenderContext::CreateBuffer(BufferType bufferType, u64 sizeBytes, int stride, Graphics::RHI_Buffer_Overrides buffer_overrides)
		{
			RHI_Buffer* buffer = m_buffers[bufferType].CreateResource();
			buffer->Create(this, bufferType, sizeBytes, stride, buffer_overrides);
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
			return m_textures.CreateResource();
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
}