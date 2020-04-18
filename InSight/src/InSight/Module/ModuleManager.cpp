#include "ModuleManager.h"

namespace Insight
{
	namespace Module
	{
		ModuleManager::ModuleManager()
		{
		}

		ModuleManager::~ModuleManager()
		{
		}

		void ModuleManager::Startup(const ModuleStartupData& startupData)
		{
		}

		void ModuleManager::Shutdown()
		{
			for (auto mod : m_modules)
			{
				delete mod.second;
			}
			m_modules.clear();
		}

		void ModuleManager::Update(const float& deltaTime)
		{
			for (auto mod : m_modules)
			{
				if (!mod.second->ShouldManuallUpate())
				{
					mod.second->Update(deltaTime);
				}
			}
		}

		bool ModuleManager::Exists(const char* moduleName)
		{
			return m_modules.find(moduleName) != m_modules.end();
		}
	}
}