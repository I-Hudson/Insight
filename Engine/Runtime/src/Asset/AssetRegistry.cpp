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

#include "Algorithm/Vector.h"

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

        for (auto& [guid, assetInfo] : m_guidToAssetInfo)
        {
            Delete(assetInfo);
        }
        m_guidToAssetInfo.clear();
        m_pathToAssetGuid.clear();

        m_state = Core::SystemStates::Not_Initialised;
    }

    AssetPackage* AssetRegistry::CreateAssetPackage(std::string_view name)
    {
        return CreateAssetPackageInternal(name, "");
    }

    AssetPackage* AssetRegistry::LoadAssetPackage(std::string_view path)
    {
        std::string pathWithExtension = FileSystem::ReplaceExtension(path, AssetPackage::c_FileExtension);
        AssetPackage* pathPackage = GetAssetPackageFromPath(pathWithExtension);
        if (pathPackage != nullptr)
        {
            return pathPackage;
        }

        if (!FileSystem::Exists(pathWithExtension))
        {
            IS_CORE_ERROR("[AssetRegistry::LoadAssetPackage] No asset package was at path '{}'.", pathWithExtension.data());
            return nullptr;
        }

        Serialisation::BinarySerialiser serialiser(true);
        serialiser.SetObjectTracking(false);
        //std::vector<Byte> fileData = FileSystem::ReadFromFile(pathWithExtension, FileType::Binary);
        //if (!serialiser.DeserialiseNoHeader(fileData))
        //{
        //    return nullptr;
        //}

        std::string packageName = FileSystem::GetFileName(pathWithExtension, true);
        AssetPackage* newPackage = CreateAssetPackageInternal(packageName, pathWithExtension);
        if (newPackage)
        {
            ::Insight::Serialisation::SerialiserObject<AssetPackage> serialiserObject;
            serialiserObject.MetaDataEnabled = false;
            serialiserObject.Deserialise(&serialiser, *newPackage);
        }

        const AssetInfo* info = GetAsset(pathWithExtension);
        if (info)
        {
            RegisterObjectToAsset(info, newPackage);
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
        ASSERT(package);

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
        /*else if (!ResourceLoaderRegister::GetLoaderFromExtension(fileExtension))
        {
            IS_CORE_ERROR("[AssetRegistry::AddAsset] Path '{}' doesn't have a compatible loader.", path.data());
            return nullptr;
        }*/

        const AssetInfo* info = GetAsset(path);
        if (info)
        {
            package->AddAsset(RemoveConst(info));
            return info;
        }

        AssetInfo* newInfo = ::New<AssetInfo>(path, package->GetPath(), package);
        m_guidToAssetInfo[newInfo->Guid] = newInfo;
        m_pathToAssetGuid[newInfo->GetFullFilePath()] = newInfo->Guid;
        package->AddAsset(newInfo);
        return newInfo;
    }

    void AssetRegistry::RemoveAsset(std::string_view path)
    {
        AssetInfo* info = RemoveConst(GetAsset(path));
        if (!info)
        {
            return;
        }

        AssetPackage* package = GetAssetPackageFromAsset(info);
        ASSERT(package);
        package->RemoveAsset(info);

        Core::GUID assetGuid = m_pathToAssetGuid.find(std::string(path))->second;
        m_pathToAssetGuid.erase(std::string(path));
        ASSERT(assetGuid.IsValid());

        AssetInfo*& assetInfo = m_guidToAssetInfo.find(assetGuid)->second;
        m_guidToAssetInfo.erase(assetGuid);

        ASSERT(assetInfo);
        Delete(assetInfo);
    }

    void AssetRegistry::UpdateMetaData(AssetInfo* assetInfo, AssetUser* object)
    {
        FAIL_ASSERT();
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
        FAIL_ASSERT();
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
        if (auto iter = m_guidToAssetInfo.find(guid);
            iter != m_guidToAssetInfo.end())
        {
            return iter->second;
        }
        return nullptr;
    }

    const AssetInfo* AssetRegistry::GetAsset(std::string_view path) const
    {
        if (auto iter = m_pathToAssetGuid.find(std::string(path));
            iter != m_pathToAssetGuid.end())
        {
            return GetAsset(iter->second);
        }
        return nullptr;
    }

    std::vector<const AssetInfo*> AssetRegistry::GetAllAssetInfos() const
    {
        std::vector<const AssetInfo*> assetInfos;
        for (const auto& [guid, assetInfo] : m_guidToAssetInfo)
        {
            assetInfos.push_back(assetInfo);
        }
        return assetInfos;
    }

    std::vector<const AssetInfo*> AssetRegistry::GetAllAssetsWithExtension(std::string_view extension) const
    {
        return GetAllAssetsWithExtensions({ extension });
    }

    std::vector<const AssetInfo*> AssetRegistry::GetAllAssetsWithExtensions(std::vector<std::string_view> extensions) const
    {
        std::vector<const AssetInfo*> assets;

        for (const auto& [guid, assetInfo] : m_guidToAssetInfo)
        {
            std::string_view assetExtension = FileSystem::GetExtension(assetInfo->FileName);
            if (!assetExtension.empty()
                && Algorithm::VectorContains(extensions, assetExtension))
            {
                assets.push_back(assetInfo);
            }
        }

        return assets;
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

        if (!FileSystem::IsDirectory(absFolderPath))
        {
            IS_CORE_ERROR("[AssetRegistry::AddAssetsInFolder] Path '{}' is not a directory.", absFolderPath);
            return;
        }

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

    AssetPackage* AssetRegistry::GetAssetPackageFromAsset(const AssetInfo* assetInfo) const
    {
        for (AssetPackage* package: m_assetPackages)
        {
            if (package->HasAsset(assetInfo)) 
            {
                return package;
            }
        }
        return nullptr;
    }

    void AssetRegistry::RegisterObjectToAsset(const AssetInfo* assetInfo, IObject* object)
    {
        std::lock_guard assetToObjectLock(m_assetToObjectGuid);
        ASSERT(m_assetToObject.find(assetInfo->Guid) == m_assetToObject.end());
        m_assetToObject[assetInfo->Guid] = object;
    }

    void AssetRegistry::UnregisterObjectToAsset(const IObject* object)
    {
        std::lock_guard assetToObjectLock(m_assetToObjectGuid);
        for (auto& [guid, obj] : m_assetToObject)
        {
            if (obj == object) 
            {
                m_assetToObject.erase(guid);
                return;
            }
        }
        FAIL_ASSERT_MSG("[AssetRegistry::UnregisterObjectToAsset] Unable to unregister object '%s' from asset.", object->GetGuid().ToString().c_str());
    }

    IObject* AssetRegistry::GetObjectFromAsset(const Core::GUID& guid)
    {
        std::lock_guard assetToObjectLock(m_assetToObjectGuid);

        if (auto iter = m_assetToObject.find(guid);
            iter != m_assetToObject.end())
        {
            return iter->second;
        }
        return nullptr;
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