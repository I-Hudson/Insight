#include "Asset/AssetRegistry.h"
#include "Asset/AssetUser.h"

#include "Runtime/ProjectSystem.h"

#include "Serialisation/Serialisers/BinarySerialiser.h"
#include "Serialisation/Serialisers/JsonSerialiser.h"

#include "FileSystem/FileSystem.h"

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
        if (HasAssetFromPath(path))
        {
            return GetAsset(path);
        }

        if (!FileSystem::Exists(path))
        {
            IS_CORE_ERROR("[AssetRegistry::AddAsset] Path '{}' doesn't exist.", path.data());
            return nullptr;
        }

        const ProjectInfo& projectInfo = ProjectSystem::Instance().GetProjectInfo();

        AssetInfo* assetInfo = New<AssetInfo>();
        assetInfo->SetFile(FileSystem::GetFileName(path), FileSystem::GetParentPath(path));

        LoadMetaData(assetInfo);

        m_guidToAssetInfoLookup[assetInfo->MetaData.AssetGuid] = assetInfo;
        m_pathToGuidLookup[path.data()] = assetInfo->MetaData.AssetGuid;

        return assetInfo;
    }

    void AssetRegistry::RemoveAsset(std::string_view path)
    {
    }

    void AssetRegistry::UpdateMetaData(AssetInfo* assetInfo, AssetUser* object)
    {
        if (!assetInfo 
            || (assetInfo && !assetInfo->IsValid())
            || (assetInfo && assetInfo->IsEngineFormat))
        {
            IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo was either null, not valid or the asset is an engine format.");
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

        const ProjectInfo& projectInfo = ProjectSystem::Instance().GetProjectInfo();
        ASSERT(FileSystem::SaveToFile(binarySerialiser.GetSerialisedData(), assetInfo->GetFullFilePath() + AssetMetaData::c_FileExtension, true));

        std::string assetPathRelativeToContent = FileSystem::GetRelativePath(assetInfo->GetFullFilePath(), projectInfo.GetContentPath());
        ASSERT(FileSystem::SaveToFile(jsonSerialiser.GetSerialisedData(), projectInfo.GetIntermediatePath() + "/meta/" + assetPathRelativeToContent + AssetMetaData::c_FileExtension, true));
    }

    void AssetRegistry::DeserialiseAssetUser(AssetUser* object) const
    {
        ASSERT(object);

        const AssetInfo* assetInfo = object->GetAssetInfo();
        if (!assetInfo
            || (assetInfo && !assetInfo->IsValid())
            || (assetInfo && assetInfo->IsEngineFormat))
        {
            IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo was either null, not valid or the asset is an engine format.");
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
        AddAssetsInFolder(path, false);
    }

    void AssetRegistry::AddAssetsInFolder(std::string_view path, bool recursive)
    {

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
        if (!assetInfo
            || (assetInfo && !assetInfo->IsValid())
            || (assetInfo && assetInfo->IsEngineFormat))
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
}