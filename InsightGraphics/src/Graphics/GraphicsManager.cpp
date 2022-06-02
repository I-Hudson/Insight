#include "Graphics/GraphicsManager.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"

#include "Core/Memory.h"
#include "Core/Profiler.h"

#include "doctest.h"

namespace Insight
{
	namespace Graphics
	{
		GraphicsManagerData GraphicsManager::m_sharedData;
		GraphicsManager* GraphicsManager::s_instance = nullptr;

		int currentGraphicsAPI;

		GraphicsManager::GraphicsManager()
		{
			assert(s_instance == nullptr);
			s_instance = this;
		}

		GraphicsManager::~GraphicsManager()
		{
			assert(s_instance != nullptr);
			s_instance = nullptr;
		}

		bool GraphicsManager::Init()
		{
			PixelFormatExtensions::Init();

			m_sharedData.GraphicsAPI = GraphicsAPI::Vulkan;
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

			m_renderpass.Create();

			return true;
		}

		void GraphicsManager::Update(const float deltaTime)
		{
			IS_PROFILE_FUNCTION();

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
					m_renderContext->GpuWaitForIdle();
					m_renderpass.Destroy();

					m_renderContext->Destroy();
					DeleteTracked(m_renderContext);
					m_renderContext = nullptr;
			
					Window::Instance().Rebuild();
			
					m_sharedData.GraphicsAPI = (GraphicsAPI)currentGraphicsAPI;
					m_renderContext = RenderContext::New();
					m_renderContext->Init();
					m_renderpass.Create();
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
				m_renderContext->GpuWaitForIdle();

				m_renderpass.Destroy();
				m_renderContext->Destroy();
				DeleteTracked(m_renderContext);
			}
		}
	}
}

#ifdef TESTING
namespace test
{
	TEST_SUITE("Create RHI Resources")
	{
		using namespace Insight;
		using namespace Insight::Graphics;
		Insight::Graphics::GraphicsManager graphicsManager;
		TEST_CASE("Create render context")
		{
			CHECK(Insight::Graphics::Window::Instance().Init());
			CHECK(graphicsManager.Init() == true);
		}

		TEST_CASE("Create Vertex RHI_Buffer")
		{
			RHI_Buffer* buffer = Renderer::CreateVertexBuffer(128, 128);
			CHECK(buffer != nullptr);

			int wantedBufferCount = Renderer::GetVertexBufferCount() - 1;
			Renderer::FreeVertexBuffer(buffer);
			CHECK(Renderer::GetVertexBufferCount() == wantedBufferCount);
		}

		TEST_CASE("Create Index RHI_Buffer")
		{
			RHI_Buffer* buffer = Renderer::CreateIndexBuffer(128);
			CHECK(buffer != nullptr);

			int wantedBufferCount = Renderer::GetIndexBufferCount() - 1;
			Renderer::FreeIndexBuffer(buffer);
			CHECK(Renderer::GetIndexBufferCount() == wantedBufferCount);
		}

		TEST_CASE("Create Uniform RHI_Buffer")
		{
			RHI_Buffer* buffer = Renderer::CreateUniformBuffer(128);
			CHECK(buffer != nullptr);

			int wantedBufferCount = Renderer::GetUniformBufferCount() - 1;
			Renderer::FreeUniformBuffer(buffer);
			CHECK(Renderer::GetUniformBufferCount() == wantedBufferCount);
		}

		TEST_CASE("Destroy render context")
		{
			graphicsManager.Destroy();
			Insight::Graphics::Window::Instance().Destroy();
		}
	}
}
#endif