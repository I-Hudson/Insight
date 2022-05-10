#include "Graphics/Renderpass.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsManager.h"

#include "Tracy.hpp"

namespace Insight
{
	namespace Graphics
	{
		void Renderpass::Render()
		{
			ZoneScoped;
			Sample();
		}

		glm::vec2 swapchainColour = { 0,0 };
		glm::vec2 swapchainColour2 = { 0,0 };

		void Renderpass::Sample()
		{
			ZoneScoped;

			static RHI_Buffer* vBuffer = nullptr;
			if (!vBuffer)
			{
				ZoneScopedN("CreateVertexBuffer");

				struct Vertex
				{
					glm::vec3 Pos;
					glm::vec3 Colour;
				};
				vBuffer = Renderer::CreateVertexBuffer(sizeof(Vertex) * 3);
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

			IMGUI_VALID(ImGui::DragFloat2("Swapchain colour", &swapchainColour.x, 0.01f, 0.0f, 1.0f));
			IMGUI_VALID(ImGui::DragFloat2("Swapchain colour2", &swapchainColour2.x, 0.01f, 0.0f, 1.0f));
			{
				ZoneScopedN("SetUniform");
				Renderer::SetUniform(0, 0, &swapchainColour, sizeof(swapchainColour));
				Renderer::SetUniform(0, 1, &swapchainColour2, sizeof(swapchainColour2));
			}
			Renderer::Draw(3, 1, 0, 0);

			{
				ZoneScopedN("FreeVertexBuffer");
				//Renderer::FreeVertexBuffer(vBuffer);
			}
		}
	}
}