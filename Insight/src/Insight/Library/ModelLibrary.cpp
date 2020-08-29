#include "ispch.h"

#include "Insight/Library/ModelLibrary.h"
#include "Insight/Memory/MemoryManager.h"

#include <filesystem>

namespace Insight
{
	namespace Library
	{
		ModelLibrary::ModelLibrary()
			: Library()
			, Serializable(this, false, "ModelLibrary.json")
		{
			SetInstancePtr(this);
		}

		ModelLibrary::~ModelLibrary()
		{
			ClearPtr();
		}

		Model* ModelLibrary::GetAssetFromPath(const std::string& filePath)
		{
			for (auto it = m_assets.begin(); it != m_assets.end(); ++it)
			{
				if ((*it).second->GetFilePath() == filePath)
				{
					return it->second;
				}
			}

			Model* m = NEW_ON_HEAP(Model, filePath);
			AddAsset(m->GetUUID(), m);

			return m;
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
					AddAsset(m->GetUUID(), m);
				}
			}
		}

		void ModelLibrary::Serialize(json& data, bool force)
		{
			for (auto it = m_assets.begin(); it != m_assets.end(); ++it)
			{
				data["Models"][it->second->GetUUID()]["UUID"] = it->second->GetUUID();
				data["Models"][it->second->GetUUID()]["FilePath"] = it->second->GetFilePath();
			}
		}

		void ModelLibrary::Deserialize(json data, bool force)
		{
			for (auto it = data["Models"].begin(); it != data["Models"].end(); ++it)
			{
				json j = *it;
				std::string uuid = (*it)["UUID"];
				Model* m = NEW_ON_HEAP(Model, (*it)["FilePath"], uuid);
				AddAsset(m->GetUUID(), m);
			}
		}
	}
}