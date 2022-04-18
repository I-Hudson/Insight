#include "Graphics/GraphicsManager.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"

#include "Core/Memory.h"

namespace Insight
{
	namespace Graphics
	{
		GraphicsManagerData GraphicsManager::m_sharedData;

		int currentGraphicsAPI;
		bool GraphicsManager::Init()
		{
			PixelFormatExtensions::Init();

			m_sharedData.GraphicsAPI = GraphicsAPI::DX12;
			currentGraphicsAPI = (int)m_sharedData.GraphicsAPI;

			m_renderContext = RenderContext::New();
			if (!m_renderContext)
			{
				return false;
			}

			if (!m_renderContext->Init())
			{
				return false;
			}

			return true;
		}

		void GraphicsManager::Update(const float deltaTime)
		{
			if (!m_renderContext)
			{
				return;
			}

			RHI_Buffer* vBuffer = Renderer::CreateVertexBuffer(128);

			bool show = true;
			std::string currentAPI = "Graphics API: " + std::to_string(currentGraphicsAPI);
			ImGui::Begin(currentAPI.c_str());
			const int previousGrapicsAPI = currentGraphicsAPI;
			const char* graphicsAPIs[] = { "Vulkan", "DX12" };
			if (ImGui::ListBox("Graphcis API", &currentGraphicsAPI, graphicsAPIs, _countof(graphicsAPIs)))
			{
				if (currentGraphicsAPI != previousGrapicsAPI)
				{
					// New API
					m_renderContext->Destroy();
					DeleteTracked(m_renderContext);
					m_renderContext = nullptr;

					Window::Instance().Rebuild();

					m_sharedData.GraphicsAPI = (GraphicsAPI)currentGraphicsAPI;
					m_renderContext = RenderContext::New();
					m_renderContext->Init();
					return;
				}
			}
			ImGui::End();

			ShaderDesc shaderDesc;
			shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
			shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
			RHI_Shader* shader = Renderer::GetShader(shaderDesc);

			PipelineStateObject pso{};
			pso.Shader = shader;
			pso.CullMode = CullMode::None;
			pso.RenderTargets.clear();
			pso.Swapchain = true;
			Renderer::SetPipelineStateObject(pso);

			Renderer::SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			Renderer::SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

			glm::vec4 swapchainColour = { 0,0,1,1 };
			Renderer::SetUniform(0,0, &swapchainColour, sizeof(swapchainColour));

			Renderer::Draw(3, 1, 0, 0);

			m_renderContext->Render(Renderer::s_FrameCommandList);
			Renderer::s_FrameCommandList.Reset();

			Renderer::FreeVertexBuffer(vBuffer);
		}

		void GraphicsManager::Destroy()
		{
			if (m_renderContext)
			{
				m_renderContext->Destroy();
				DeleteTracked(m_renderContext);
				m_renderContext = nullptr;
			}
		}
	}
}