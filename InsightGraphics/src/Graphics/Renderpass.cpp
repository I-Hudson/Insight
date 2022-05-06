#include "Graphics/Renderpass.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsManager.h"

#include "Tracy.hpp"

namespace Insight
{
	namespace Graphics
	{
		void VulkanTest()
		{
			ShaderDesc shaderDesc;
			shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
			shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
			RHI_Shader* shader = Renderer::GetShader(shaderDesc);

			PipelineStateObject pso{};
			pso.Name = L"Swapchain_PSO";
			pso.Shader = shader;
			pso.CullMode = CullMode::None;
			pso.RenderTargets.clear();
			pso.Swapchain = true;
			Renderer::SetPipelineStateObject(pso);

			Renderer::SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			Renderer::SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

			Renderer::Draw(3, 1, 0, 0);
		}

		void Renderpass::Render()
		{
			ZoneScoped;
			if (GraphicsManager::IsVulkan())
			{
				VulkanTest();
			}
			else
			{
				Sample();
			}
		}

		glm::vec4 swapchainColour = { 0,0,1,1 };

		void Renderpass::Sample()
		{
			ZoneScoped;

			RHI_Buffer* vBuffer = nullptr;
			{
				ZoneScopedN("CreateVertexBuffer");
				//vBuffer = Renderer::CreateVertexBuffer(128);
			}

			RHI_Shader* shader = nullptr;
			{
				ZoneScopedN("GetShader");
				ShaderDesc shaderDesc;
				shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
				shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
				shader = Renderer::GetShader(shaderDesc);
			}

			PipelineStateObject pso{};
			{
				ZoneScopedN("SetPipelineStateObject");
				pso.Name = L"Swapchain_PSO";
				pso.Shader = shader;
				pso.CullMode = CullMode::None;
				pso.RenderTargets.clear();
				pso.Swapchain = true;
				Renderer::SetPipelineStateObject(pso);
			}

			Renderer::SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			Renderer::SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

			IMGUI_VALID(ImGui::DragFloat4("Swapchain colour", &swapchainColour.x, 0.01f, 0.0f, 1.0f));
			{
				ZoneScopedN("SetUniform");
				Renderer::SetUniform(0, 0, &swapchainColour, sizeof(swapchainColour));
			}
			Renderer::Draw(3, 1, 0, 0);

			{
				ZoneScopedN("FreeVertexBuffer");
				Renderer::FreeVertexBuffer(vBuffer);
			}
		}
	}
}