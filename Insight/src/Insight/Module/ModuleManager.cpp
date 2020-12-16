#include "ispch.h"

#include "ModuleManager.h"
#include "Insight/Instrumentor/Instrumentor.h"

namespace Insight
{
	namespace Module
	{
		ModuleManager::ModuleManager()
		{
		}

		ModuleManager::~ModuleManager()
		{
			for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
			{
				if (it->second != nullptr && it->second->GetDependenciesCount() == 0)
				{
					SharedPtr<Module> m = it->second;
					m->~Module();
					m.reset();

					m_modules.erase(it);
					it = m_modules.begin();
				}
				else if (it != m_modules.end())
				{
					++it;
				}
				else
				{
					IS_CORE_ASSERT(false, "Not all modules have been removed.");
				}
			}
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

		bool ModuleManager::Exists(const std::string& moduleName)
		{
			return m_modules.find(moduleName) != m_modules.end();
		}
	}
}