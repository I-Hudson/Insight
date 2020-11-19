#include "ispch.h"
#include "GraphicsModule.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Module/WindowModule.h"
#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Renderer/ImGuiRenderer.h"

#include "Insight/Time/Stopwatch.h"
#include "Insight/Log.h"

namespace Insight
{
	namespace Module
	{
		CameraComponent* GraphicsModule::m_mainCamera;
		std::vector<MeshComponent*> GraphicsModule::m_meshs;

		GraphicsModule::GraphicsModule(WindowModule* windowModule) 
		{
			SetInstancePtr(this);

			m_windowModule = windowModule;

			m_renderer = Renderer::Create();

			ImGuiRenderer::Create();
			ImGuiRenderer::Instance()->Init(m_renderer);
		}

		GraphicsModule::~GraphicsModule()
		{
			m_renderer->WaitForIdle();

			Insight::ImGuiRenderer* imguiRenderer = Insight::ImGuiRenderer::Instance();
			DELETE_ON_HEAP(imguiRenderer);

			DELETE_ON_HEAP(m_renderer);
			m_windowModule = nullptr;

			ClearPtr();
		}

		void GraphicsModule::Update(const float& deltaTime)
		{
			IS_PROFILE_FUNCTION();

			if (m_renderer != nullptr)
			{
				//m_renderer->Clear();

				Insight::Stopwatch stopwatch;

				stopwatch.Start();
				m_renderer->Render(m_mainCamera, m_meshs);
				stopwatch.End();

				std::stringstream ss;
				double fps = 1.0 / stopwatch.Sec();
				ss << "FPS: " << fps;
				//Insight::Window::SetTitle(ss.str());


				//m_renderer->Present();
			}
			else
			{
				IS_CORE_ERROR("[GraphicsModule::Update] No renderer setup.");
			}
		}

		void GraphicsModule::SetMainCamera(CameraComponent* camera)
		{
			m_mainCamera = camera;
		}

		const bool GraphicsModule::HasMainCamera()
		{
			return m_mainCamera != nullptr;
		}

		const bool GraphicsModule::IsThisMainCamera(CameraComponent* camera)
		{
			return camera == m_mainCamera;
		}

		Material* GraphicsModule::GetDefaultMaterial()
		{
			return GraphicsModule::Instance()->m_renderer->GetDefaultMaterial();
		}
	}
}