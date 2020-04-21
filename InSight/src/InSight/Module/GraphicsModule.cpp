#include "ispch.h"
#include "GraphicsModule.h"

#include "Insight/Module/WindowModule.h"

#include "Insight/Log.h"

namespace Insight
{
	namespace Module
	{
		GraphicsModule::GraphicsModule()
		{
		}

		GraphicsModule::~GraphicsModule()
		{
			m_windowModule = nullptr;
		}

		void GraphicsModule::Startup(ModuleStartupData startupData)
		{
			GraphicsModuleStartupData data = static_cast<GraphicsModuleStartupData&>(startupData);
			m_windowModule = data.WindowModule;

			IS_CORE_INFO("{0}", m_windowModule->GetWindow()->GetHeight());
		}

		void GraphicsModule::Shutdown()
		{
		}

		void GraphicsModule::Update(const float& deltaTime)
		{
		}
	}
}