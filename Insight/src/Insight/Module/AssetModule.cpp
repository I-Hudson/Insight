#include "ispch.h"

#include "Insight/Module/AssetModule.h"
#include "Insight/Memory/MemoryManager.h"

#include "Insight/Model/Model.h"

namespace Insight
{
	namespace Module
	{
		AssetModule::AssetModule()
			: m_deserlizaed(false)
		{
			m_modelLibrary = CreateSharedPtr<Insight::Library::ModelLibrary>();
			//m_shaderLibrary = NEW_ON_HEAP(Insight::Library::ShaderLibrary);
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
			//m_deserlizaed = true;
			//tinyxml2::XMLDocument doc;
			//if (doc.LoadFile("ModelLibrary.xml") == tinyxml2::XML_SUCCESS)
			//{
			//	m_modelLibrary->Deserialize(doc.FirstChild());
			//}

			IS_PROFILE_START_CAPTURE();
			m_modelLibrary->LoadAssetsFromFolder("./data", true);
			IS_PROFILE_STOP_CAPTURE();
			IS_PROFILE_SAVE_CAPTURE("ModelLoadCapture.opt");
		}
	}
}