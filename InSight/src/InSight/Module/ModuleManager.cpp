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
			for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
			{
				Module* m = *it;
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
				if (!mod->ShouldManuallUpate())
				{
					mod->Update(deltaTime);
				}
			}
		}

		bool ModuleManager::Exists(const char* moduleName)
		{
			for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
			{
				if (typeid(*it).name() == moduleName)
				{
					return true;
				}
			}
			return false;
		}
	}
}