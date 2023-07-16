#include "Asset/AssetRegistry.h"
#include "Asset/AssetUser.h"

#include "Runtime/ProjectSystem.h"

#include "Resource/Loaders/ResourceLoaderRegister.h"
#include "Resource/ResourceDatabase.h"

#include "Serialisation/Serialisers/BinarySerialiser.h"
#include "Serialisation/Serialisers/JsonSerialiser.h"

#include "FileSystem/FileSystem.h"

#include "Core/EnginePaths.h"
#include "Core/Logger.h"

namespace Insight::Runtime
{
    void AssetRegistry::Initialise()
    {
        m_state = Core::SystemStates::Initialised;
    }

    void AssetRegistry::Shutdown()
    {
        for (auto& [guid, assetInfo] : m_guidToAssetInfoLookup)
        {
            ::Delete(assetInfo);
        }
        m_guidToAssetInfoLookup = {};
        m_pathToGuidLookup = {};

        m_state = Core::SystemStates::Not_Initialised;
    }

    const AssetInfo* AssetRegistry::AddAsset(std::string_view path)
    {
        return AddAsset(path, true);
    }

    const AssetInfo* AssetRegistry::AddAsset(std::string_view path, bool enableMetaFile)
    {
        if (HasAssetFromPath(path))
        {
            return GetAsset(path);
        }

        std::string_view fileExtension = FileSystem::GetFileExtension(path);
        if (!FileSystem::Exists(path))
        {
            IS_CORE_ERROR("[AssetRegistry::AddAsset] Path '{}' doesn't exist.", path.data());
            return nullptr;
        }
        else if (!FileSystem::IsFile(path))
        {
            return nullptr;
        }
        else if (FileSystem::GetFileExtension(path) == AssetMetaData::c_FileExtension
            || FileSystem::GetFileExtension(path) == ResourceDatabase::c_MetaFileExtension
            || FileSystem::GetFileExtension(path) == ".meta")
        {
            return nullptr;
        }
        else if (!ResourceLoaderRegister::GetLoaderFromExtension(fileExtension))
        {
            IS_CORE_ERROR("[AssetRegistry::AddAsset] Path '{}' doesn't have a compatible loader.", path.data());
            return nullptr;
        }

        AssetInfo* assetInfo = New<AssetInfo>();
        assetInfo->SetFile(FileSystem::GetFileName(path), FileSystem::GetParentPath(path));
        assetInfo->EnableMetaData = enableMetaFile;

        LoadMetaData(assetInfo);

        m_guidToAssetInfoLookup[assetInfo->MetaData.AssetGuid] = assetInfo;
        m_pathToGuidLookup[assetInfo->GetFullFilePath()] = assetInfo->MetaData.AssetGuid;

        return assetInfo;
    }

    void AssetRegistry::RemoveAsset(std::string_view path)
    {
        if (!HasAssetFromPath(path))
        {
            return;
        }

        AssetInfo* assetInfo = RemoveConst(GetAsset(path));
        ASSERT(assetInfo);
        ASSERT(assetInfo->GetCount() == 0);

        m_guidToAssetInfoLookup.erase(assetInfo->MetaData.AssetGuid);
        m_pathToGuidLookup.erase(assetInfo->GetFullFilePath());
        ::Delete(assetInfo);
    }

    void AssetRegistry::UpdateMetaData(AssetInfo* assetInfo, AssetUser* object)
    {
        if (!AssetInfoValidate(assetInfo))
        {
            return;
        }

        Serialisation::BinarySerialiser binarySerialiser(false);
        Serialisation::JsonSerialiser jsonSerialiser(false);

        AssetMetaData& metaData = assetInfo->MetaData;

        metaData.Serialise(&binarySerialiser);
        if (object)
        {
            object->Serialise(&binarySerialiser);
        }

        metaData.Serialise(&jsonSerialiser);
        if (object)
        {
            object->Serialise(&jsonSerialiser);
        }

        ASSERT(FileSystem::SaveToFile(binarySerialiser.GetSerialisedData(), assetInfo->GetFullFilePath() + AssetMetaData::c_FileExtension, true));

        if (FileSystem::PathIsSubPathOf(assetInfo->GetFullFilePath(), EnginePaths::GetResourcePath()))
        {
            std::string assetPathRelativeToContent = FileSystem::GetRelativePath(assetInfo->GetFullFilePath(), EnginePaths::GetResourcePath());
            ASSERT(FileSystem::SaveToFile(jsonSerialiser.GetSerialisedData(), EnginePaths::GetResourcePath() + "/meta/" + assetPathRelativeToContent + AssetMetaData::c_FileExtension, true));
        }
        else
        {
            const ProjectInfo& projectInfo = ProjectSystem::Instance().GetProjectInfo();
            if (FileSystem::PathIsSubPathOf(assetInfo->GetFullFilePath(), projectInfo.GetContentPath()))
            {
                std::string assetPathRelativeToContent = FileSystem::GetRelativePath(assetInfo->GetFullFilePath(), projectInfo.GetContentPath());
                ASSERT(FileSystem::SaveToFile(jsonSerialiser.GetSerialisedData(), projectInfo.GetIntermediatePath() + "/meta/" + assetPathRelativeToContent + AssetMetaData::c_FileExtension, true));
            }
            else
            {
                FAIL_ASSERT();
            }
        }
    }

    void AssetRegistry::DeserialiseAssetUser(AssetInfo* assetInfo, AssetUser* object) const
    {
        ASSERT(object);

        if (!AssetInfoValidate(assetInfo))
        {
            return;
        }

        Serialisation::BinarySerialiser binarySerialiser(true);

        std::string metaFilePath = assetInfo->GetFullFilePath() + AssetMetaData::c_FileExtension;
        std::vector<Byte> fileData = FileSystem::ReadFromFile(metaFilePath, FileType::Binary);

        if (!binarySerialiser.Deserialise(fileData))
        {
            return;
        }

        //AssetMetaData metaData;
        //metaData.Deserialise(&binarySerialiser);
        // Skip the AssetMetaData.
        binarySerialiser.SkipObject();
        if (!binarySerialiser.AtEnd())
        {
            object->Deserialise(&binarySerialiser);
        }
        object->SetAssetInfo(assetInfo);
    }

    const AssetInfo* AssetRegistry::GetAsset(const Core::GUID& guid) const
    {
        const auto iter = m_guidToAssetInfoLookup.find(guid);
        if (iter == m_guidToAssetInfoLookup.end())
        {
            return nullptr;
        }
        return iter->second;
    }

    const AssetInfo* AssetRegistry::GetAsset(std::string_view path) const
    {
        const auto iter = m_pathToGuidLookup.find(std::string(path));
        if (iter == m_pathToGuidLookup.end())
        {
            return nullptr;
        }
        return GetAsset(iter->second);
    }

    void AssetRegistry::AddAssetsInFolder(std::string_view path)
    {
        AddAssetsInFolder(path, false, true);
    }

    void AssetRegistry::AddAssetsInFolder(std::string_view path, bool recursive)
    {
        AddAssetsInFolder(path, recursive, true);
    }

    void AssetRegistry::AddAssetsInFolder(std::string_view path, bool recursive, bool enableMetaFiles)
    {
        std::string absFolderPath = FileSystem::GetAbsolutePath(path);
        if (recursive)
        {
            for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(absFolderPath))
            {
                std::string path = entry.path().string();
                FileSystem::PathToUnix(path);
                AddAsset(path, enableMetaFiles);
            }
        }
        else
        {
            for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(absFolderPath))
            {
                std::string path = entry.path().string();
                FileSystem::PathToUnix(path);
                AddAsset(path, enableMetaFiles);
            }
        }
    }

    bool AssetRegistry::HasAssetFromGuid(const Core::GUID& guid) const
    {
        return GetAsset(guid) != nullptr;
    }
    
    bool AssetRegistry::HasAssetFromPath(std::string_view path) const
    {
        return GetAsset(path) != nullptr;
    }

    void AssetRegistry::LoadMetaData(AssetInfo* assetInfo)
    {
        if (!AssetInfoValidate(assetInfo))
        {
            return;
        }

        Serialisation::BinarySerialiser binarySerialiser(true);

        std::string metaFilePath = assetInfo->GetFullFilePath() + AssetMetaData::c_FileExtension;
        if (!FileSystem::Exists(metaFilePath))
        {
            UpdateMetaData(assetInfo, nullptr);
        }

        std::vector<Byte> fileData = FileSystem::ReadFromFile(metaFilePath, FileType::Binary);

        if (!binarySerialiser.Deserialise(fileData))
        {
            return;
        }

        AssetMetaData& metaData = RemoveConst(assetInfo)->MetaData;
        metaData.Deserialise(&binarySerialiser);
    }

    bool AssetRegistry::AssetInfoValidate(const AssetInfo* assetInfo) const
    {
        if (!assetInfo)
        {
            IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo is null.");
            return false;
        }
        else if (!assetInfo->EnableMetaData)
        {
            IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo has MetaData disabled. No data is stored on disk for this asset.");
            return false;
        }
        else if (!assetInfo->IsValid())
        {
            IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo is not valid.");
            return false;
        }
        else if (assetInfo->IsEngineFormat)
        {
            IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo is an engine format.");
            return false;
        }
        return true;
    }
}