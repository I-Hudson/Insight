#include "ispch.h"

#include "ModuleManager.h"
#include "Insight/Instrumentor/Instrumentor.h"

namespace Insight
{
	namespace Module
	{
		ModuleManager::ModuleManager(ModuleStartupData& startupData)
			: Module(startupData)
		{
		}

		ModuleManager::~ModuleManager()
		{
			for (std::unordered_map<const char*, Module*>::iterator it = m_modules.end(); it != m_modules.begin();)
			{
				--it;
				Module* m = it->second;
				m->~Module();
				DELETE_ON_STACK(m);
			}
			// No need to call delete as all modules are placed on the stack.
			m_modules.clear();
		}

		void ModuleManager::Update(const float& deltaTime)
		{
			IS_PROFILE_FUNCTION();

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