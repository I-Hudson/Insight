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

        AssetInfo* assetInfo = New<AssetInfo>();
        assetInfo->SetFile(path.data());

        LoadMetaData(assetInfo);

        m_guidToAssetInfoLookup[assetInfo->MetaData.AssetGuid] = assetInfo;
        m_pathToGuidLookup[path.data()] = assetInfo->MetaData.AssetGuid;

        return assetInfo;
    }

    void AssetRegistry::RemoveAsset(std::string_view path)
    {
    }

    void AssetRegistry::UpdateMetaData(AssetUser* object)
    {
        if (object == nullptr)
        {
            return;
        }

        const AssetInfo* assetInfo = object->GetAssetInfo();
        if (!assetInfo 
            || (assetInfo && !assetInfo->IsValid())
            || (assetInfo && assetInfo->IsEngineFormat))
        {
            IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo was either null, not valid or the asset is an engine format.");
            return;
        }

        Serialisation::BinarySerialiser binarySerialiser(false);
        Serialisation::JsonSerialiser jsonSerialiser(false);

        AssetMetaData& metaData = RemoveConst(assetInfo)->MetaData;

        metaData.Serialise(&binarySerialiser);
        object->Serialise(&binarySerialiser);

        metaData.Serialise(&jsonSerialiser);
        object->Serialise(&jsonSerialiser);

        FileSystem::SaveToFile(binarySerialiser.GetSerialisedData(), assetInfo->GetFullFilePath() + AssetMetaData::c_FileExtension);
        FileSystem::SaveToFile(jsonSerialiser.GetSerialisedData(), "debug/meta/" + assetInfo->FileName + AssetMetaData::c_FileExtension);
    }

    const AssetInfo* AssetRegistry::GetAsset(const Core::GUID& guid) const
    {
        const auto iter = m_guidToAssetInfoLookup.find(guid);
        if (iter == m_guidToAssetInfoLookup.end())
        {
            return false;
        }
        return iter->second;
    }

    const AssetInfo* AssetRegistry::GetAsset(std::string_view path) const
    {
        const auto iter = m_pathToGuidLookup.find(std::string(path));
        if (iter == m_pathToGuidLookup.end())
        {
            return false;
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
        std::vector<Byte> fileData = FileSystem::ReadFromFile(metaFilePath, FileType::Binary);

        if (!binarySerialiser.Deserialise(fileData))
        {
            return;
        }

        AssetMetaData& metaData = RemoveConst(assetInfo)->MetaData;
        metaData.Deserialise(&binarySerialiser);
    }
}