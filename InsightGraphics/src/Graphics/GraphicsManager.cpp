#include "Graphics/GraphicsManager.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"

#include "Core/Memory.h"

#include "Tracy.hpp"

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
			ZoneScoped;

			if (!m_renderContext)
			{
				return;
			}

			bool show = true;
			std::string currentAPI = "Graphics API: " + std::to_string(currentGraphicsAPI);
			const int previousGrapicsAPI = currentGraphicsAPI;
			const char* graphicsAPIs[] = { "Vulkan", "DX12" };
			IMGUI_VALID(ImGui::Begin(currentAPI.c_str()));
			IMGUI_VALID(if (ImGui::ListBox("Graphcis API", &currentGraphicsAPI, graphicsAPIs, _countof(graphicsAPIs)))
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
					Renderer::s_FrameCommandList.Reset();
					return;
				}
			})
			IMGUI_VALID(ImGui::End());

			m_renderpass.Render();
			m_renderContext->Render(Renderer::s_FrameCommandList);
			Renderer::s_FrameCommandList.Reset();
		}

		void GraphicsManager::Destroy()
		{
			if (m_renderContext)
			{
				m_renderContext->Destroy();
				DeleteTracked(m_renderContext);
			}
		}
	}
}