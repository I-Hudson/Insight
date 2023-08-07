#include "Asset/AssetRegistry.h"

#include "Asset/AssetUser.h"
#include "Asset/AssetPackage.h"

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
        for (AssetPackage*& package : m_assetPackages)
        {
            Delete(package);
        }
        m_assetPackages.clear();

        m_state = Core::SystemStates::Not_Initialised;
    }

    AssetPackage* AssetRegistry::CreateAssetPackage(std::string_view name)
    {
        return CreateAssetPackageInternal(name, "");
    }

    AssetPackage* AssetRegistry::LoadAssetPackage(std::string_view path)
    {
        AssetPackage* pathPackage = GetAssetPackageFromPath(path);
        if (pathPackage != nullptr)
        {
            return pathPackage;
        }

        if (!FileSystem::Exists(path))
        {
            IS_CORE_ERROR("[AssetRegistry::LoadAssetPackage] No asset package was at path '{}'.", path.data());
            return nullptr;
        }

        Serialisation::BinarySerialiser serialiser(true);
        std::vector<Byte> fileData = FileSystem::ReadFromFile(path, FileType::Binary);
        if (!serialiser.DeserialiseNoHeader(fileData))
        {
            return nullptr;
        }

        std::string packageName = FileSystem::GetFileName(path, true);
        AssetPackage* newPackage = CreateAssetPackageInternal(packageName, path);
        if (newPackage)
        {
            newPackage->Deserialise(&serialiser);
        }
        return newPackage;
    }

    void AssetRegistry::UnloadAssetPackage(std::string_view path)
    {
    }

    void AssetRegistry::SetDebugDirectories(std::string metaFileDirectory, std::string assetReativeBaseDirectory)
    {
        if (!FileSystem::Exists(metaFileDirectory))
        {
            FileSystem::CreateFolder(metaFileDirectory);
        }
        m_debugMetaFileDirectory = std::move(metaFileDirectory);
        m_assetReativeBaseDirectory = std::move(assetReativeBaseDirectory);
    }

    const AssetInfo* AssetRegistry::AddAsset(std::string_view path, AssetPackage* package)
    {
        return AddAsset(path, package, true);
    }

    const AssetInfo* AssetRegistry::AddAsset(std::string_view path, AssetPackage* package, bool enableMetaFile)
    {
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

        return package->AddAsset(path);
    }

    void AssetRegistry::RemoveAsset(std::string_view path, AssetPackage* package)
    {
        package->RemoveAsset(path);
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

        //metaData.Serialise(&binarySerialiser);
        if (object)
        {
            object->Serialise(&binarySerialiser);
        }

        //metaData.Serialise(&jsonSerialiser);
        if (object)
        {
            object->Serialise(&jsonSerialiser);
        }

        ASSERT(FileSystem::SaveToFile(binarySerialiser.GetSerialisedData(), assetInfo->GetFullFilePath() + AssetMetaData::c_FileExtension,FileType::Binary, true));

        if (!m_debugMetaFileDirectory.empty())
        {
            std::string assetPathRelativeToContent = FileSystem::GetRelativePath(assetInfo->GetFullFilePath(), m_assetReativeBaseDirectory);
            ASSERT(FileSystem::SaveToFile(jsonSerialiser.GetSerialisedData(), m_debugMetaFileDirectory + "/" + assetPathRelativeToContent + AssetMetaData::c_FileExtension, true));

        }
    }

    void AssetRegistry::UpdateMetaData(AssetUser* object)
    {
        UpdateMetaData(RemoveConst(object->GetAssetInfo()), object);
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

    std::vector<Byte> AssetRegistry::LoadAsset(std::string_view path) const
    {
        const AssetInfo* info = GetAsset(path);
        if (info && info->AssetPackage)
        {
            return info->AssetPackage->LoadAsset(path);
        }
        IS_CORE_ERROR("[AssetRegistry::LoadAsset] Unable to load asset from path '{}'.", path.data());
        return {};
    }

    const AssetInfo* AssetRegistry::GetAsset(const Core::GUID& guid) const
    {
        for (const AssetPackage* package : m_assetPackages)
        {
            const AssetInfo* assetInfo = package->GetAsset(guid);
            if (assetInfo)
            {
                return assetInfo;
            }
        }
        return nullptr;
    }

    const AssetInfo* AssetRegistry::GetAsset(std::string_view path) const
    {
        for (const AssetPackage* package : m_assetPackages)
        {
            const AssetInfo* assetInfo = package->GetAsset(path);
            if (assetInfo)
            {
                return assetInfo;
            }
        }
        return nullptr;
    }

    std::vector<const AssetInfo*> AssetRegistry::GetAllAssetInfos() const
    {
        std::vector<const AssetInfo*> assetInfos;
        for (const AssetPackage* package : m_assetPackages)
        {
            std::vector<const AssetInfo*> packageAssetInfos = package->GetAllAssetInfos();
            std::move(packageAssetInfos.begin(), packageAssetInfos.end(), std::back_inserter(assetInfos));
        }
        return assetInfos;
    }

    AssetPackage* AssetRegistry::GetAssetPackageFromPath(std::string_view path) const
    {
        if (path.empty())
        {
            return nullptr;
        }

        for (AssetPackage* package : m_assetPackages)
        {
            if (package->GetPath() == path)
            {
                return package;
            }
        }
        return nullptr;
    }

    AssetPackage* AssetRegistry::GetAssetPackageFromName(std::string_view name) const
    {
        for (AssetPackage* package : m_assetPackages)
        {
            if (package->GetName() == name)
            {
                return package;
            }
        }
        return nullptr;
    }

    std::vector<AssetPackage*> AssetRegistry::GetAllAssetPackages() const
    {
        return m_assetPackages;
    }

    void AssetRegistry::AddAssetsInFolder(std::string_view path, AssetPackage* package)
    {
        AddAssetsInFolder(path, package, false, true);
    }

    void AssetRegistry::AddAssetsInFolder(std::string_view path, AssetPackage* package, bool recursive)
    {
        AddAssetsInFolder(path, package, recursive, true);
    }

    void AssetRegistry::AddAssetsInFolder(std::string_view path, AssetPackage* package, bool recursive, bool enableMetaFiles)
    {
        std::string absFolderPath = FileSystem::GetAbsolutePath(path);
        if (recursive)
        {
            for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(absFolderPath))
            {
                std::string path = entry.path().string();
                FileSystem::PathToUnix(path);
                AddAsset(path, package, enableMetaFiles);
            }
        }
        else
        {
            for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(absFolderPath))
            {
                std::string path = entry.path().string();
                FileSystem::PathToUnix(path);
                AddAsset(path, package, enableMetaFiles);
            }
        }
    }

    AssetPackage* AssetRegistry::CreateAssetPackageInternal(std::string_view name, std::string_view path)
    {
        AssetPackage* pathPackage = GetAssetPackageFromPath(name);
        AssetPackage* namePackage = GetAssetPackageFromName(name);
        if (pathPackage && namePackage
            && pathPackage == namePackage)
        {
            // Package is on disk.
            return pathPackage;
        }
        else if (namePackage)
        {
            // Package is not on disk.
            ASSERT(pathPackage == nullptr);
            return namePackage;
        }
        ASSERT(pathPackage == nullptr && namePackage == nullptr);

        AssetPackage* newPackage = New<AssetPackage>(path, name);
        m_assetPackages.push_back(newPackage);
        return newPackage;
    }

    bool AssetRegistry::HasAssetFromGuid(const Core::GUID& guid) const
    {
        return GetAsset(guid) != nullptr;
    }
    
    bool AssetRegistry::HasAssetFromPath(std::string_view path) const
    {
        return GetAsset(path) != nullptr;
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