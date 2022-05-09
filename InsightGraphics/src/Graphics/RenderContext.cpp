#include "Graphics/RenderContext.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"

#include "backends/imgui_impl_glfw.h"
#include "Core/Memory.h"

namespace Insight
{
	Graphics::CommandList Renderer::s_FrameCommandList;
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
			context->m_shaderManager.SetRenderContext(context);
			context->m_descriptorLayoutManager.SetRenderContext(context);

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

		bool RenderContext::HasExtension(DeviceExtension extension)
		{
			return m_deviceExtensions[(u32)extension] == 1;
		}

		void RenderContext::ImGuiBeginFrame()
		{
			IMGUI_VALID(ImGui_ImplGlfw_NewFrame());
			IMGUI_VALID(ImGui::NewFrame());
			IMGUI_VALID(ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode));
		}

		void RenderContext::ImGuiRender()
		{
			IMGUI_VALID(ImGui::Render());
			IMGUI_VALID(ImGui::UpdatePlatformWindows());
		}

		void RenderContext::BaseDestroy()
		{
			m_vertexBuffer.ReleaseAll();
			m_descriptorLayoutManager.ReleaseAll();
			m_shaderManager.Destroy();
		}

		RHI_Buffer* RenderContext::CreateBuffer(BufferType bufferType, u64 sizeBytes)
		{
			RHI_Buffer* buffer = m_buffers[bufferType].CreateResource();
			buffer->Create(this, bufferType, sizeBytes);
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
	}

	
	// Renderer
	void Renderer::SetImGUIContext(ImGuiContext*& context)
	{
		context = ImGui::GetCurrentContext();
	}

	Graphics::RHI_Buffer* Renderer::CreateVertexBuffer(u64 sizeBytes)
	{
		return s_context->CreateBuffer(Graphics::BufferType::Vertex, sizeBytes);
	}

	Graphics::RHI_Buffer* Renderer::CreateIndexBuffer(u64 sizeBytes)
	{
		return s_context->CreateBuffer(Graphics::BufferType::Index, sizeBytes);
	}

	Graphics::RHI_Buffer* Renderer::CreateUniformBuffer(u64 sizeBytes)
	{
		return s_context->CreateBuffer(Graphics::BufferType::Uniform, sizeBytes);
	}

	Graphics::RHI_Buffer* Renderer::CreateRawBuffer(u64 sizeBytes)
	{
		return s_context->CreateBuffer(Graphics::BufferType::Raw, sizeBytes);
	}

	void Renderer::FreeVertexBuffer(Graphics::RHI_Buffer* buffer)
	{
		s_context->FreeBuffer(buffer);
	}

	void Renderer::FreeIndexBuffer(Graphics::RHI_Buffer* buffer)
	{
		assert(buffer->GetType() == Graphics::BufferType::Vertex);
		s_context->FreeBuffer(buffer);
	}

	void Renderer::FreeUniformBuffer(Graphics::RHI_Buffer* buffer)
	{
		assert(buffer->GetType() == Graphics::BufferType::Uniform);
		s_context->FreeBuffer(buffer);
	}

	void Renderer::FreeRawBuffer(Graphics::RHI_Buffer* buffer)
	{
		assert(buffer->GetType() == Graphics::BufferType::Index);
		s_context->FreeBuffer(buffer);
	}

	void Renderer::BindVertexBuffer(Graphics::RHI_Buffer* buffer)
	{
		assert(buffer->GetType() == Graphics::BufferType::Raw);
		s_FrameCommandList.SetVertexBuffer(buffer);
	}

	void Renderer::BindIndexBuffer(Graphics::RHI_Buffer* buffer)
	{
		s_FrameCommandList.SetIndexBuffer(buffer);
	}

	Graphics::RHI_Shader* Renderer::GetShader(Graphics::ShaderDesc desc)
	{
		return s_context->m_shaderManager.GetOrCreateShader(desc);
	}

	void Renderer::SetPipelineStateObject(Graphics::PipelineStateObject pso)
	{
		s_FrameCommandList.SetPipelineStateObject(pso);
	}

	void Renderer::SetViewport(int width, int height)
	{
		s_FrameCommandList.SetViewport(width, height);
	}

	void Renderer::SetScissor(int width, int height)
	{
		s_FrameCommandList.SetScissor(width, height);
	}

	void Renderer::SetUniform(int set, int binding, void* data, int sizeInBytes)
	{
		s_FrameCommandList.SetUniform(set, binding, data, sizeInBytes);
	}

	void Renderer::SetTexture(int set, int binding, Graphics::RHI_Texture* texture)
	{
	}

	void Renderer::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
	{
		s_FrameCommandList.Draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void Renderer::DrawIndexed()
	{
		s_FrameCommandList.DrawIndexed(0, 0, 0, 0, 0);
	}
}