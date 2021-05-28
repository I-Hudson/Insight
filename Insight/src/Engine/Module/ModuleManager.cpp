#include "ispch.h"

#include "Engine/Module/ModuleManager.h"
#include "Engine/Instrumentor/Instrumentor.h"

namespace Insight::Module
{
	ModuleManager::ModuleManager()
	{
	}

	ModuleManager::~ModuleManager()
	{
		for (auto& mod : m_modules)
		{
			if (mod != nullptr && !mod->ShouldDestroyManually())
			{
				::Delete(mod);
			}
		}
		m_modules.clear();
	}

	void ModuleManager::Update(const float& deltaTime)
	{
		IS_PROFILE_FUNCTION();

		for (auto& mod : m_modules)
		{
			if (!mod->ShouldManuallUpate())
			{
				mod->Update(deltaTime);
			}
		}
	}

	bool ModuleManager::Exists(const Type& type)
	{
		return m_typeToModule.find(type) != m_typeToModule.end();
	}
}