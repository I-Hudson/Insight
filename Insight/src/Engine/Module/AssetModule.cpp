#include "ispch.h"

#include "Engine/Module/AssetModule.h"
#include "Engine/Memory/MemoryManager.h"

#include "Engine/Model/Model.h"

	namespace Module
	{
		AssetModule::AssetModule()
			: m_deserlizaed(false)
		{
			m_modelLibrary = ::New<Library::ModelLibrary>();
			//m_shaderLibrary = NEW_ON_HEAP(Library::ShaderLibrary);
		}

		AssetModule::~AssetModule()
		{
			::Delete(m_modelLibrary);
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