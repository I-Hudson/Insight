#include "Graphics/Renderpass.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsManager.h"

#include "optick.h"

namespace Insight
{
	namespace Graphics
	{
		void Renderpass::Create()
		{
			m_testMesh.LoadFromFile("./Resources/models/sponza_old/sponza.obj");

			if (!m_vertexBuffer)
			{
				//ZoneScopedN("CreateVertexBuffer");

				Vertex vertices[3] =
				{
					Vertex( glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f),	glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) ),
					Vertex( glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),	glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) ),
					Vertex( glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),	glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) ),
				};
				m_vertexBuffer = Renderer::CreateVertexBuffer(sizeof(vertices), sizeof(Vertex));
				m_vertexBuffer->Upload(vertices, sizeof(vertices));
				m_vertexBuffer->SetName(L"TriangleMesh_Vertex_Buffer");
			}

			if (!m_indexBuffer)
			{
				int indices[3] = { 0, 1, 2, };
				m_indexBuffer = Renderer::CreateIndexBuffer(sizeof(int) * ARRAYSIZE(indices));
				m_indexBuffer->Upload(indices, sizeof(indices));
				m_indexBuffer->SetName(L"TriangleMesh_Index_Buffer");
			}
		}

		void Renderpass::Render()
		{
			OPTICK_EVENT();
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

			m_testMesh.Destroy();
		}

		glm::vec2 swapchainColour = { 0,0 };
		glm::vec2 swapchainColour2 = { 0,0 };

		void Renderpass::Sample()
		{
			OPTICK_EVENT();

			RHI_Shader* shader = nullptr;
			{
				//ZoneScopedN("GetShader");
				ShaderDesc shaderDesc;
				shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
				shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
				shader = Renderer::GetShader(shaderDesc);
			}

			PipelineStateObject pso{};
			{
				//ZoneScopedN("SetPipelineStateObject");
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
				//ZoneScopedN("SetUniform");
				Renderer::SetUniform(0, 0, &swapchainColour, sizeof(swapchainColour));
				Renderer::SetUniform(0, 1, &swapchainColour2, sizeof(swapchainColour2));
			}

			m_testMesh.Draw();

			//Renderer::Draw(3, 1, 0, 0);
			//Renderer::DrawIndexed(3, 1, 0, 0, 0);
		}
	}
}