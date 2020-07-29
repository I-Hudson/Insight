#include "ispch.h"

#include "Insight/Module/AssetModule.h"
#include "Insight/Memory/MemoryManager.h"

namespace Insight
{
	namespace Module
	{
		AssetModule::AssetModule(ModuleStartupData& startupData)
		{
			m_modelLibrary = NEW_ON_HEAP(ModelLibrary);
			m_shaderLibrary = NEW_ON_HEAP(ShaderLibrary);
		}

		AssetModule::~AssetModule()
		{
			DELETE_ON_HEAP(m_modelLibrary);
			DELETE_ON_HEAP(m_shaderLibrary);
		}

		void AssetModule::Update(const float& deltaTime)
		{
		}
	}
}