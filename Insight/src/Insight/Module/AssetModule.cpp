#include "ispch.h"

#include "Insight/Module/AssetModule.h"
#include "Insight/Memory/MemoryManager.h"

#include "Insight/Model/Model.h"

	namespace Module
	{
		AssetModule::AssetModule()
			: m_deserlizaed(false)
		{
			m_modelLibrary = CreateSharedPtr<Library::ModelLibrary>();
			//m_shaderLibrary = NEW_ON_HEAP(Library::ShaderLibrary);
		}

		AssetModule::~AssetModule()
		{
			m_modelLibrary.reset();
			//DELETE_ON_HEAP(m_shaderLibrary);
		}

		void AssetModule::Update(const float& deltaTime)
		{
		}

		void AssetModule::Deserialize()
		{
			IS_PROFILE_START_CAPTURE();
			m_modelLibrary->LoadAssetsFromFolder("./data", true);
			IS_PROFILE_STOP_CAPTURE();
			IS_PROFILE_SAVE_CAPTURE("ModelLoadCapture");
		}
	}