#include "ispch.h"
#include "GraphicsModule.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Module/WindowModule.h"

#include "Insight/Log.h"

namespace Insight
{
	namespace Module
	{
		GraphicsModule::GraphicsModule(ModuleStartupData& startupData) 
			: Module(startupData)
		{
			GraphicsModuleStartupData data = static_cast<GraphicsModuleStartupData&>(startupData);
			m_windowModule = data.WindowModule;

			IS_CORE_INFO("{0}", m_windowModule->GetWindow()->GetHeight());

			RendererStartUpData renderData{ m_windowModule };
			m_renderer = Renderer::Create(renderData);
		}

		GraphicsModule::~GraphicsModule()
		{
			Memory::MemoryManager::DeleteOnFreeList(m_renderer);
			m_windowModule = nullptr;
		}

		void GraphicsModule::Update(const float& deltaTime)
		{
			m_renderer->Clear();

			m_renderer->Render();

			m_renderer->Present();
		}
	}
}