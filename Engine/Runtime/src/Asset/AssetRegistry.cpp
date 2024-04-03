#include "Asset/AssetRegistry.h"

#include "Asset/AssetPackage/IAssetPackage.h"
#include "Asset/AssetPackage/AssetPackageFileSystem.h"
#include "Asset/AssetPackage/AssetPackageZip.h"

#include "Asset/Importers/ModelImporter.h"
#include "Asset/Importers/TextureImporter.h"

#include "Runtime/ProjectSystem.h"

#include "Resource/Loaders/ResourceLoaderRegister.h"
#include "Resource/ResourceDatabase.h"

#include "Serialisation/Serialisers/BinarySerialiser.h"
#include "Serialisation/Serialisers/JsonSerialiser.h"

#include "FileSystem/FileSystem.h"

#include "Core/EnginePaths.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Threading/ScopedLock.h"

#include "Algorithm/Vector.h"

namespace Insight::Runtime
{
    void AssetRegistry::Initialise()
    {
        m_importers.push_back(New<ModelImporter>());
        m_importers.push_back(New<TextureImporter>());

        m_state = Core::SystemStates::Initialised;
    }

    void AssetRegistry::Shutdown()
    {
        IS_PROFILE_FUNCTION();

        for (IAssetPackage*& package : m_assetPackages)
        {
            Delete(package);
            package = nullptr;
        }

        for (auto& [path, assetInfo] : m_pathToAssetInfo)
        {
            Delete(assetInfo);
        }

        for (IAssetImporter*& importer : m_importers)
        {
            Delete(importer);
        }

        m_state = Core::SystemStates::Not_Initialised;
    }

    IAssetPackage* AssetRegistry::CreateAssetPackage(std::string_view name)
    {
        return CreateAssetPackageInternal(name, "", AssetPackageType::FileSystem);
    }

    IAssetPackage* AssetRegistry::LoadAssetPackage(std::string_view path)
    {
        std::string pathWithExtension = std::string(path);// FileSystem::ReplaceExtension(path, IAssetPackage::c_FileExtension);
        pathWithExtension = FileSystem::GetAbsolutePath(pathWithExtension);
        IAssetPackage* pathPackage = GetAssetPackageFromPath(pathWithExtension);
        if (pathPackage != nullptr)
        {
            return pathPackage;
        }

        if (!FileSystem::Exists(pathWithExtension))
        {
            IS_CORE_ERROR("[AssetRegistry::LoadAssetPackage] No asset package was at path '{}'.", pathWithExtension.data());
            return nullptr;
        }

        AssetPackageType packageType = FileSystem::GetExtension(pathWithExtension) == IAssetPackage::c_FileExtension
            || FileSystem::GetExtension(pathWithExtension) == ".zip" ? AssetPackageType::Zip : AssetPackageType::FileSystem;

        std::string packageName = FileSystem::GetFileName(pathWithExtension, true);
        IAssetPackage* newPackage = CreateAssetPackageInternal(packageName, pathWithExtension, packageType);

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

    const AssetInfo* AssetRegistry::AddAssetFromPackage(std::string_view path, IAssetPackage* package)
    {
        return AddAsset(path, package, true, false);
    }

    const AssetInfo* AssetRegistry::AddAssetFromDisk(std::string_view path, IAssetPackage* package)
    {
        return AddAsset(path, package, true, true);
    }

    const AssetInfo* AssetRegistry::AddAsset(std::string_view path, IAssetPackage* package, bool enableMetaFile, bool checkOnDisk)
    {
        ASSERT(package);

        std::string_view fileExtension = FileSystem::GetFileExtension(path);
        if (checkOnDisk)
        {
            if (!FileSystem::Exists(path))
            {
                IS_CORE_ERROR("[AssetRegistry::AddAsset] Path '{}' doesn't exist.", path.data());
                return nullptr;
            }
            else if (!FileSystem::IsFile(path))
            {
                return nullptr;
            }
            else if (FileSystem::GetFileExtension(path) == ResourceDatabase::c_MetaFileExtension
                || FileSystem::GetFileExtension(path) == ".meta")
            {
                return nullptr;
            }
            /*else if (!ResourceLoaderRegister::GetLoaderFromExtension(fileExtension))
            {
                IS_CORE_ERROR("[AssetRegistry::AddAsset] Path '{}' doesn't have a compatible loader.", path.data());
                return nullptr;
            }*/
        }

        const AssetInfo* info = GetAsset(path);
        if (info)
        {
            package->AddAsset(RemoveConst(info));
            return info;
        }

        AssetInfo* newInfo = ::New<AssetInfo>(path, package->GetPath(), package, enableMetaFile);
        Core::MemoryTracker::Instance().NameAllocation(newInfo, path.data());

        m_pathToAssetInfo[newInfo->GetFullFilePath()] = newInfo;
        const AssetInfo* addedInfo = package->AddAsset(newInfo);
        ASSERT(newInfo == addedInfo);

        return newInfo;
    }

    void AssetRegistry::RemoveAsset(std::string_view path)
    {
        AssetInfo* info = RemoveConst(GetAsset(path));
        if (!info)
        {
            return;
        }

        IAssetPackage* package = GetAssetPackageFromAsset(info);
        if (package)
        { 
            package->RemoveAsset(info);
        }

        AssetInfo* assetInfo = m_pathToAssetInfo.find(std::string(path))->second;
        m_pathToAssetInfo.erase(std::string(path));

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
    
        AssetMetaData* metaData = assetInfo->MetaData;
        ASSERT(metaData)
    
        metaData->Serialise(&binarySerialiser);
        if (object)
        {
            //object->Serialise(&binarySerialiser);
        }
    
        metaData->Serialise(&jsonSerialiser);
        if (object)
        {
            //object->Serialise(&jsonSerialiser);
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
        //UpdateMetaData(RemoveConst(object->GetAssetInfo()), object);
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
            //object->Deserialise(&binarySerialiser);
        }
        //object->SetAssetInfo(assetInfo);
    }

    Ref<Asset> AssetRegistry::LoadAsset2(std::string path)
    {
        /*
            Order of things:
            First get the correct asset importer for the data being loaded. Do this before loading the data
            so we don't read from disk, then just discard the data, saving on IO processing and remove possible not needed
            newing and deleteing from vector creation.

            Then check that we have a valid AssetInfo for this path.

            If we have a valid importer and valid AssetInfo then we can load the byte data from disk.
            // TODO When loading data maybe the importer should be in charge as then it might not have to load the whole contents 
            of the file into a single buffer but could seek and only load the parts in wants at only one point in time.
        */

        if (path.empty())
        {
            return Ref<Asset>();
        }

        path = ValidatePath(path);

        const IAssetImporter* importer = nullptr;
        std::string_view extension = FileSystem::GetExtension(path);
        for (const IAssetImporter* assetImporter : m_importers)
        {
            if (assetImporter && assetImporter->IsValidImporterForFileExtension(extension.data()))
            {
                importer = assetImporter;
                break;
            }
        }
        if (importer == nullptr)
        {
            IS_CORE_ERROR("[AssetRegistry::LoadAsset2] 'Importer' is nullptr for extension '{}'.", extension);
            return Ref<Asset>();
        }

        const AssetInfo* assetInfo = GetAssetInfo(path);
        if (assetInfo == nullptr)
        {
            IS_CORE_ERROR("[AssetRegistry::LoadAsset2] Unable to get AssetInfo from path '{}'.", path);
            return Ref<Asset>();
        }

        Ref<Asset> asset = importer->Import(assetInfo, path);
        {
            Threading::ScopedLock lock(m_loadedAssetLock);
            m_loadedAssets[path] = asset;
        }
        return asset;
    }

    const AssetInfo* AssetRegistry::GetAssetInfo(const std::string& path) const
    {
#define GET_ASSET_INFO_DIRECT_FROM_ASSET_PACKAGE
#ifdef GET_ASSET_INFO_DIRECT_FROM_ASSET_PACKAGE
        for (size_t i = 0; i < m_assetPackages.size(); ++i)
        {
            const AssetInfo* assetInfo = m_assetPackages[i]->GetAsset(path);
            if (assetInfo != nullptr)
            {
                return assetInfo;
            }
        }
#else
        if (auto iter = m_pathToAssetInfo.find(std::string(path));
            iter != m_pathToAssetInfo.end())
        {
            return iter->second;
        }
#endif
#undef GET_ASSET_INFO_DIRECT_FROM_ASSET_PACKAGE
        return nullptr;
    }

    std::vector<Byte> AssetRegistry::LoadAssetData(std::string_view path) const
    {
        IS_PROFILE_FUNCTION();

        std::string absPath = FileSystem::GetAbsolutePath(path);
        const AssetInfo* info = GetAsset(absPath);
        if (info && info->AssetPackage)
        {
            return info->AssetPackage->LoadAsset(absPath);
        }
        else
        {
            if (FileSystem::Exists(absPath))
            {
                return FileSystem::ReadFromFile(absPath);
            }
        }
        IS_CORE_ERROR("[AssetRegistry::LoadAsset] Unable to load asset from path '{}'.", absPath.data());
        return {};
    }

    std::vector<Byte> AssetRegistry::LoadAsset(std::string_view path) const
    {
        std::string absPath = FileSystem::GetAbsolutePath(path);
        const AssetInfo* info = GetAsset(absPath);
        if (info && info->AssetPackage)
        {
            return info->AssetPackage->LoadAsset(absPath);
        }
        else
        {
            if (FileSystem::Exists(absPath))
            {
               return FileSystem::ReadFromFile(absPath);
            }
        }
        IS_CORE_ERROR("[AssetRegistry::LoadAsset] Unable to load asset from path '{}'.", absPath.data());
        return {};
    }

    Ref<Asset> AssetRegistry::GetAsset2(const std::string& path) const
    {
        return Ref<Asset>();
    }

    const AssetInfo* AssetRegistry::GetAsset(const Core::GUID& guid) const
    {
        for (const auto& [path, assetInfo] : m_pathToAssetInfo)
        {
            if (guid == assetInfo->Guid)
            {
                return assetInfo;
            }
        }
        return nullptr;
    }

    const AssetInfo* AssetRegistry::GetAsset(std::string_view path) const
    {
        if (auto iter = m_pathToAssetInfo.find(std::string(path));
            iter != m_pathToAssetInfo.end())
        {
            return iter->second;
        }
        return nullptr;
    }

    std::vector<const AssetInfo*> AssetRegistry::GetAllAssetInfos() const
    {
        std::vector<const AssetInfo*> assetInfos;
        for (const auto& [path, assetInfo] : m_pathToAssetInfo)
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

        for (const auto& [path, assetInfo] : m_pathToAssetInfo)
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

    IAssetPackage* AssetRegistry::GetAssetPackageFromPath(std::string_view path) const
    {
        if (path.empty())
        {
            return nullptr;
        }

        for (IAssetPackage* package : m_assetPackages)
        {
            if (package && package->GetPath() == path)
            {
                return package;
            }
        }
        return nullptr;
    }

    IAssetPackage* AssetRegistry::GetAssetPackageFromName(std::string_view name) const
    {
        for (IAssetPackage* package : m_assetPackages)
        {
            if (package && package->GetName() == name)
            {
                return package;
            }
        }
        return nullptr;
    }

    std::vector<IAssetPackage*> AssetRegistry::GetAllAssetPackages() const
    {
        return m_assetPackages;
    }

    void AssetRegistry::AddAssetsInFolder(std::string_view path, IAssetPackage* package)
    {
        AddAssetsInFolder(path, package, false, true);
    }

    void AssetRegistry::AddAssetsInFolder(std::string_view path, IAssetPackage* package, bool recursive)
    {
        AddAssetsInFolder(path, package, recursive, true);
    }

    void AssetRegistry::AddAssetsInFolder(std::string_view path, IAssetPackage* package, bool recursive, bool enableMetaFiles)
    {
        std::string absFolderPath = FileSystem::GetAbsolutePath(path);

        if (!FileSystem::IsDirectory(absFolderPath))
        {
            IS_CORE_ERROR("[AssetRegistry::AddAssetsInFolder] Path '{}' is not a directory.", absFolderPath);
            return;
        }

        std::vector<std::string> assetPaths;
        if (recursive)
        {
            for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(absFolderPath))
            {
                std::string path = entry.path().string();
                FileSystem::PathToUnix(path);
                assetPaths.push_back(path);
            }
        }
        else
        {
            for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(absFolderPath))
            {
                std::string path = entry.path().string();
                FileSystem::PathToUnix(path);
                assetPaths.push_back(path);
            }
        }

        std::sort(assetPaths.begin(), assetPaths.end(), [](const std::string& strA, const std::string& strB)
            {
                return FileSystem::GetExtension(strA) == Runtime::AssetMetaData::c_FileExtension
                    && FileSystem::GetExtension(strB) != Runtime::AssetMetaData::c_FileExtension;
            });

        for (const std::string& path : assetPaths)
        {
            AddAsset(path, package, enableMetaFiles, true);
        }
    }

    IAssetPackage* AssetRegistry::GetAssetPackageFromAsset(const AssetInfo* assetInfo) const
    {
        for (IAssetPackage* package: m_assetPackages)
        {
            if (package && package->HasAsset(assetInfo))
            {
                return package;
            }
        }
        return nullptr;
    }

    void AssetRegistry::RegisterObjectToAsset(const AssetInfo* assetInfo, IObject* object)
    {
        std::lock_guard assetToObjectLock(m_assetToObjectGuid);
        std::unordered_set<const IObject*>& objects = m_assetToObjects[assetInfo->Guid];
        ASSERT(objects.find(object) == objects.end());
        m_assetToObjects[assetInfo->Guid].insert(object);
    }

    void AssetRegistry::UnregisterObjectToAsset(const IObject* object)
    {
        std::lock_guard assetToObjectLock(m_assetToObjectGuid);
        for (auto& [guid, set] : m_assetToObjects)
        {
            if (auto iter = set.find(object);
                iter != set.end())
            {
                set.erase(object);
                return;
            }
        }
        FAIL_ASSERT_MSG("[AssetRegistry::UnregisterObjectToAsset] Unable to unregister object '%s' from asset.", object->GetGuid().ToString().c_str());
    }

    std::vector<IObject*> AssetRegistry::GetObjectsFromAsset(const Core::GUID& guid) const
    {
        std::lock_guard assetToObjectLock(m_assetToObjectGuid);

        std::vector<IObject*> objects;
        if (auto iter = m_assetToObjects.find(guid);
            iter != m_assetToObjects.end())
        {
            std::transform(iter->second.begin(), iter->second.end(), std::back_inserter(objects), [](const IObject* obj)
                {
                    return RemoveConst(obj);
                });
        }
        return objects;
    }

    IAssetPackage* AssetRegistry::CreateAssetPackageInternal(std::string_view name, std::string_view path, AssetPackageType packageType)
    {
        IAssetPackage* pathPackage = GetAssetPackageFromPath(name);
        IAssetPackage* namePackage = GetAssetPackageFromName(name);
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

        IAssetPackage* newPackage = nullptr;

        switch(packageType)
        {
        case AssetPackageType::FileSystem:
            newPackage = New<AssetPackageFileSystem>(path, name);
            break;
        case AssetPackageType::Zip:
            newPackage = New<AssetPackageZip>(path, name);

            Serialisation::BinarySerialiser serialiser(true);
            serialiser.SetObjectTracking(false);
            std::vector<Byte> fileData = FileSystem::ReadFromFile(path, FileType::Binary);
            //ASSERT_MSG(serialiser.DeserialiseNoHeader(fileData), "Unable to load asset package zip.");

            ::Insight::Serialisation::SerialiserObject<AssetPackageZip> serialiserObject;
            serialiserObject.MetaDataEnabled = false;
            serialiserObject.Deserialise(&serialiser, *static_cast<AssetPackageZip*>(newPackage));

            break;   
        }
        
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

    std::string AssetRegistry::ValidatePath(const std::string& path) const
    {
        if (path.empty())
        {
            return path;
        }

        if (FileSystem::IsAbsolutePath(path))
        {
            return path;
        }
        else
        {
            std::string newPath = ProjectSystem::Instance().GetProjectInfo().GetContentPath();
            if (path.front() != '/')
            {
                newPath += '/';
            }
            newPath += path;
            return newPath;
        }
    }
}