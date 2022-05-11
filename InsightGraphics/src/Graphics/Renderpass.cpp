#include "Graphics/Renderpass.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsManager.h"

#include "Tracy.hpp"

namespace Insight
{
	namespace Graphics
	{
		void Renderpass::Create()
		{
			if (!m_vertexBuffer)
			{
				ZoneScopedN("CreateVertexBuffer");

				struct Vertex
				{
					glm::vec3 Pos;
					glm::vec3 Colour;
				};

				Vertex vertices[3] =
				{
					Vertex{ glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f) },
					Vertex{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) },
					Vertex{ glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) }
				};
				m_vertexBuffer = Renderer::CreateVertexBuffer(sizeof(Vertex) * ARRAYSIZE(vertices));
				m_vertexBuffer->Upload(vertices, sizeof(vertices));

			}

			if (!m_indexBuffer)
			{
				int indices[3] = { 0, 1, 2, };
				m_indexBuffer = Renderer::CreateIndexBuffer(sizeof(int) * ARRAYSIZE(indices));
				m_indexBuffer->Upload(indices, sizeof(indices));
			}
		}

		void Renderpass::Render()
		{
			ZoneScoped;
			Sample();
		}

		void Renderpass::Destroy()
		{
			if (m_vertexBuffer)
			{
				Renderer::FreeVertexBuffer(m_vertexBuffer);
				m_vertexBuffer = nullptr;
			}

			if (m_indexBuffer)
			{
				Renderer::FreeIndexBuffer(m_indexBuffer);
				m_indexBuffer = nullptr;
			}
		}

		glm::vec2 swapchainColour = { 0,0 };
		glm::vec2 swapchainColour2 = { 0,0 };

		void Renderpass::Sample()
		{
			ZoneScoped;

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

			Renderer::BindVertexBuffer(m_vertexBuffer);
			Renderer::BindIndexBuffer(m_indexBuffer);

			IMGUI_VALID(ImGui::DragFloat2("Swapchain colour", &swapchainColour.x, 0.01f, 0.0f, 1.0f));
			IMGUI_VALID(ImGui::DragFloat2("Swapchain colour2", &swapchainColour2.x, 0.01f, 0.0f, 1.0f));
			{
				ZoneScopedN("SetUniform");
				Renderer::SetUniform(0, 0, &swapchainColour, sizeof(swapchainColour));
				Renderer::SetUniform(0, 1, &swapchainColour2, sizeof(swapchainColour2));
			}
			//Renderer::Draw(3, 1, 0, 0);
			Renderer::DrawIndexed(3, 1, 0, 0, 0);
		}
	}
}