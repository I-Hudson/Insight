#include "ispch.h"

#include "Engine/Module/ModuleManager.h"
#include "Engine/Instrumentor/Instrumentor.h"

	namespace Module
	{
		ModuleManager::ModuleManager()
		{
		}

		ModuleManager::~ModuleManager()
		{
			for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
			{
				if (it->second != nullptr && !it->second->ShouldDestroyManually())
				{
					::Delete(it->second);
				}
			}
			m_modules.clear();
		}

		void ModuleManager::Update(const float& deltaTime)
		{
			IS_PROFILE_FUNCTION();

			for (auto& mod : m_modules)
			{
				if (!mod.second->ShouldManuallUpate())
				{
					mod.second->Update(deltaTime);
				}
			}
		}

		bool ModuleManager::Exists(const Type& type)
		{
			return m_modules.find(type) != m_modules.end();
		}
	}