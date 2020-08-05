#include "ispch.h"

#include "Insight/Module/AssetModule.h"
#include "Insight/Memory/MemoryManager.h"

#include "Insight/Assimp/Model.h"
#include "Insight/Renderer/Shader.h"

namespace Insight
{
	namespace Module
	{
		AssetModule::AssetModule()
			: m_deserlizaed(false)
		{
			m_modelLibrary = NEW_ON_HEAP(Insight::Library::ModelLibrary);
			m_shaderLibrary = NEW_ON_HEAP(Insight::Library::ShaderLibrary);
		}

		AssetModule::~AssetModule()
		{
			DELETE_ON_HEAP(m_modelLibrary);
			DELETE_ON_HEAP(m_shaderLibrary);
		}

		void AssetModule::Update(const float& deltaTime)
		{
		}

		void AssetModule::Deserialize()
		{
			m_deserlizaed = true;
			std::ifstream in;
			in.open("ModelLibrary.json");
			json j;
			in >> j;
			m_modelLibrary->Deserialize(j);
			in.close();
		}
	}
}