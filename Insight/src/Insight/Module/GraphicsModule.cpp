#include "ispch.h"
#include "GraphicsModule.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Module/WindowModule.h"
#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Component/MeshComponent.h"

#include "Insight/Log.h"

namespace Insight
{
	namespace Module
	{
		Camera* GraphicsModule::m_mainCamera;
		std::vector<MeshComponent*> GraphicsModule::m_meshs;

		GraphicsModule::GraphicsModule(WindowModule* windowModule) 
		{
			SetInstancePtr(this);

			m_windowModule = windowModule;

			IS_CORE_INFO("{0}", m_windowModule->GetWindow()->GetHeight());

			RendererStartUpData renderData{ m_windowModule };
			m_renderer = Renderer::Create(renderData);
		}

		GraphicsModule::~GraphicsModule()
		{
			Memory::MemoryManager::DeleteOnFreeList(m_renderer);
			m_windowModule = nullptr;
			
			ClearPtr();
		}

		void GraphicsModule::Update(const float& deltaTime)
		{
			IS_PROFILE_FUNCTION();

			m_renderer->Clear();

			m_renderer->Render(m_mainCamera, m_meshs);

			m_renderer->Present();
		}

		void GraphicsModule::SetMainCamera(Camera* camera)
		{
			m_mainCamera = camera;
		}

		Material* GraphicsModule::GetDefaultMaterial()
		{
			return GraphicsModule::GetInstance()->m_renderer->GetDefaultMaterial();
		}
	}
}