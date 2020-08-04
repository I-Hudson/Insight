#include "ispch.h"

#include "Insight/Library/ModelLibrary.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Assimp/Model.h"

#include <filesystem>

namespace Insight
{
	namespace Library
	{
		ModelLibrary::ModelLibrary()
			: Library()
			, Serializable(this, false, "ModelLibrary.json")
		{
		}

		ModelLibrary::~ModelLibrary()
		{
		}

		void ModelLibrary::LoadAssetsFromFolder(const std::string& folderName, const bool& lookInChildren)
		{
			using fs = std::filesystem::recursive_directory_iterator;

			for (const auto& entry : std::filesystem::recursive_directory_iterator::recursive_directory_iterator(folderName))
			{
				std::string extension = entry.path().u8string();
				extension = extension.substr(extension.find_last_of("."));

				auto exIt = std::find(m_extensions.begin(), m_extensions.end(), extension);
				if (exIt != m_extensions.end())
				{
					Model* m = NEW_ON_HEAP(Model, entry.path().u8string());
					Insight::UUID* uuid = static_cast<Insight::UUID*>(m);
					AddAsset(uuid->GetUUID(), m);
				}
			}
		}

		void ModelLibrary::Serialize(json& data)
		{
			for (auto it = m_assets.begin(); it != m_assets.end(); ++it)
			{
				data["Models"][it->second->GetUUID()] = it->second->GetFilePath();
			}
		}

		void ModelLibrary::Deserialize(json data)
		{

		}
	}
}