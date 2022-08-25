#include "Graphics/RenderContext.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"

#include "Graphics/RenderTarget.h"

#include "Core/Memory.h"

#include "backends/imgui_impl_glfw.h"

namespace Insight
{
	Graphics::RenderContext* Renderer::s_context;
	
#define ENABLE_IMGUI 1

	namespace Graphics
	{
		RenderContext* RenderContext::New()
		{
			RenderContext* context = nullptr;
			if (GraphicsManager::IsVulkan()) { context = NewTracked(RHI::Vulkan::RenderContext_Vulkan); }
			else if (GraphicsManager::IsDX12()) { context = NewTracked(RHI::DX12::RenderContext_DX12); }

			if (!context)
			{
				IS_CORE_ERROR("[RenderContext* RenderContext::New] Unable to create a RenderContext.");
				return context;
			}

			::Insight::Renderer::s_context = context;
			context->m_samplerManager = RHI_SamplerManager::New();
			
			context->m_descriptorSetManager.Setup();
			context->m_commandListManager.Setup();

			context->m_shaderManager.SetRenderContext(context);
			context->m_descriptorLayoutManager.SetRenderContext(context);
			context->m_renderpassManager.SetRenderContext(context);
			context->m_samplerManager->SetRenderContext(context);

			if (ENABLE_IMGUI)
			{
				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
				//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
				io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
				//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
				//io.ConfigViewportsNoAutoMerge = true;
				//io.ConfigViewportsNoTaskBarIcon = true;

				// Setup Dear ImGui style
				ImGui::StyleColorsDark();
				//ImGui::StyleColorsClassic();

				// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
				ImGuiStyle& style = ImGui::GetStyle();
				if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					style.WindowRounding = 0.0f;
					style.Colors[ImGuiCol_WindowBg].w = 1.0f;
				}
			}

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
			IMGUI_VALID(ImGui_ImplGlfw_NewFrame());
			IMGUI_VALID(ImGui::NewFrame());
			IMGUI_VALID(ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode));

			IMGUI_VALID(RenderStats::Instance().Draw());
		}

		void RenderContext::ImGuiRender()
		{
			IMGUI_VALID(ImGui::Render());
			IMGUI_VALID(ImGui::UpdatePlatformWindows());
		}

		void RenderContext::BaseDestroy()
		{
			for (auto& buffer : m_buffers)
			{
				buffer.second.ReleaseAll();
			}
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
		}

		RHI_Buffer* RenderContext::CreateBuffer(BufferType bufferType, u64 sizeBytes, int stride)
		{
			RHI_Buffer* buffer = m_buffers[bufferType].CreateResource();
			buffer->Create(this, bufferType, sizeBytes, stride);
			return buffer;
		}

		void RenderContext::FreeBuffer(RHI_Buffer* buffer)
		{
			if (buffer)
			{
				BufferType bufferType = buffer->GetType();
				m_buffers[bufferType].FreeResource(buffer);
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
			m_textures.FreeResource(texture);
		}

		void FrameResouce::Reset()
		{
			CommandListManager.Update();
			UniformBuffer.Reset();
		}
	}

	// Renderer
	void Renderer::SetImGUIContext(ImGuiContext*& context)
	{
		context = ImGui::GetCurrentContext();
	}

	Graphics::RHI_Buffer* Renderer::CreateVertexBuffer(u64 sizeBytes, int stride)
	{
		ASSERT(s_context);
		return s_context->CreateBuffer(Graphics::BufferType::Vertex, sizeBytes, stride);
	}

	Graphics::RHI_Buffer* Renderer::CreateIndexBuffer(u64 sizeBytes)
	{
		ASSERT(s_context);
		return s_context->CreateBuffer(Graphics::BufferType::Index, sizeBytes, 0);
	}

	Graphics::RHI_Buffer* Renderer::CreateUniformBuffer(u64 sizeBytes)
	{
		ASSERT(s_context);
		return s_context->CreateBuffer(Graphics::BufferType::Uniform, sizeBytes, 0);
	}

	Graphics::RHI_Buffer* Renderer::CreateRawBuffer(u64 sizeBytes)
	{
		return s_context->CreateBuffer(Graphics::BufferType::Raw, sizeBytes, 0);
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
		ASSERT(buffer->GetType() == Graphics::BufferType::Index);
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
}