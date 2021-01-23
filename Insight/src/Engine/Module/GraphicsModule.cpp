#include "ispch.h"
#include "GraphicsModule.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Module/WindowModule.h"
#include "Engine/Instrumentor/Instrumentor.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Graphics/ImGuiRenderer.h"

#include "Engine/Time/Stopwatch.h"
#include "Engine/Core/Log.h"

	namespace Module
	{
		CameraComponent* GraphicsModule::m_mainCamera;
		std::vector<WeakPtr<MeshComponent>> GraphicsModule::m_meshs;

		GraphicsModule::GraphicsModule(SharedPtr<WindowModule> windowModule)
		{
			m_windowModule = windowModule;

			m_renderer = Renderer::Create();
			m_renderer->OnCreate();

			m_imguiRenderer = ImGuiRenderer::Create();
			ImGuiRenderer::Instance()->Init(m_renderer);
		}

		GraphicsModule::~GraphicsModule()
		{
			ImGuiRenderer* imguiRenderer = ImGuiRenderer::Instance();
			m_imguiRenderer.reset();

			m_renderer.reset();
			m_windowModule = nullptr;
		}

		void GraphicsModule::Update(const float& deltaTime)
		{
			IS_PROFILE_FUNCTION();

			if (m_renderer != nullptr)
			{
				m_renderer->Render(m_mainCamera, m_meshs);
			}
			else
			{
				IS_CORE_ERROR("[GraphicsModule::Update] No renderer setup.");
			}
		}

		void GraphicsModule::WaitForIdle()
		{
			m_renderer->WaitForIdle();
		}

		GraphicsAPI GraphicsModule::GetAPI()
		{
			return m_renderer->GetAPI();
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