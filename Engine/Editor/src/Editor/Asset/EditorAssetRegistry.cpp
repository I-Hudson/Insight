#include "Editor/Asset/EditorAssetRegistry.h"

#include "Event/EventSystem.h"
#include "Core/Logger.h"
#include "Serialisation/Serialisers/BinarySerialiser.h"
#include "Serialisation/Serialisers/JsonSerialiser.h"

#include "FileSystem/FileSystem.h"
#include "Runtime/ProjectSystem.h"
#include "Asset/Importers/IAssetImporter.h"

#include "Asset/Assets/Texture.h"

#include <filesystem>

namespace Insight
{
	namespace Editor
	{
		IS_SERIALISABLE_CPP(TextureImportSettings);


		using Serialiser = Serialisation::BinarySerialiser;

		EditorAssetRegistry::EditorAssetRegistry()
		{ }

		EditorAssetRegistry::~EditorAssetRegistry()
		{ }

		void EditorAssetRegistry::Initialise()
		{
			Runtime::AssetRegistry::Initialise();
		}

		void EditorAssetRegistry::Shutdown()
		{
#if 0
			{
				std::lock_guard lock(m_cachedAssetLock);
				for (auto& [path, asset] : m_cachedAssets)
				{
					AssetRegistry::UnloadAsset(asset);
					asset.Reset();
				}
			}
			m_cachedAssets.clear();
#endif //0
			Runtime::AssetRegistry::Shutdown();
		}

		void EditorAssetRegistry::InitialiseCache()
		{
			ASSERT(Runtime::ProjectSystem::Instance().IsProjectOpen());

			const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
			m_cacheRootFolder = projectInfo.GetIntermediatePath() + "/CachedAssets";

			//GetAllCachedAssets();
		}

		Ref<Runtime::Asset> EditorAssetRegistry::LoadAsset(std::string path)
		{
			ValidatePath(path);

			if (!Runtime::ProjectSystem::Instance().IsProjectOpen())
			{
				return Runtime::AssetRegistry::LoadAssetInternal(path, false);
			}

			const Runtime::AssetInfo* assetInfo = GetAssetInfo(path);
			if (!assetInfo)
			{
				IS_LOG_CORE_WARN("[EditorAssetRegistry::LoadAsset] No AssetInfo for '{}'.", path.c_str());
				return Ref<Runtime::Asset>();
			}

			Ref<Runtime::Asset> asset = LoadCachedAsset(assetInfo);
			if (asset)
			{
				return asset;
			}

			const Runtime::IAssetImporter* importer = AssetRegistry::GetImporter(FileSystem::GetExtension(path));
			if (!importer || !importer->CanConvertToEngineFormat())
			{
				return Runtime::AssetRegistry::LoadAssetInternal(path, false);
			}

			asset = Runtime::AssetRegistry::LoadAssetInternal(path, true);
			if (!asset)
			{
				IS_LOG_CORE_WARN("[EditorAssetRegistry::LoadAsset] Loaded asset is null at path '{}'.", path.c_str());
				return Ref<Runtime::Asset>();
			}

			{
				std::lock_guard lock(m_loadedAssetLock);
				m_loadedAssets[assetInfo->GetFullFilePath()] = asset;
			}

			//TEMP
			return asset;

			ASSERT(asset->GetAssetInfo() == assetInfo);

			Serialiser serialiser;
			asset->Serialise(&serialiser);

			const std::string filePath = m_cacheRootFolder + "/" + assetInfo->Guid.ToString() + asset->GetAssetFileExtension();
			FileSystem::SaveToFile(serialiser.GetSerialisedData(), filePath);

			return asset;
		}

		Ref<Runtime::Asset> EditorAssetRegistry::LoadCachedAsset(const Runtime::AssetInfo* assetInfo)
		{
			{
				std::lock_guard lock(m_loadedAssetLock);
				if (const auto iter = m_loadedAssets.find(assetInfo->GetFullFilePath());
					iter != m_loadedAssets.end())
				{
					return iter->second;
				}
			}

			std::string fileExtenion;
			if (assetInfo->MetaData->ReflectType == Runtime::TextureAsset::GetStaticTypeInfo().GetType())
			{
				fileExtenion = Runtime::TextureAsset::GetStaticAssetFileExtension();
			}

			const std::string cachedAssetPath = std::string(GetCachedRootFolder()) + "/" + assetInfo->Guid.ToString() + fileExtenion;

			Ref<Runtime::Asset> asset;
			if (FileSystem::Exists(cachedAssetPath))
			{
				const Runtime::IAssetImporter* importer = AssetRegistry::GetImporter(FileSystem::GetExtension(cachedAssetPath));
				asset = importer->CreateAsset(assetInfo);
				
				{
					std::lock_guard lock(m_loadedAssetLock);
					m_loadedAssets[assetInfo->GetFullFilePath()] = asset;
				}

				const std::vector<u8> diskData = Runtime::AssetRegistry::LoadAssetData(cachedAssetPath);
				Serialiser serialiser(true);
				serialiser.Deserialise(diskData);

				asset->Deserialise(&serialiser);

			}
			return asset;
		}
	}
}