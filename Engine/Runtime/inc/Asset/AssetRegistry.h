#pragma once

#include "Runtime/Defines.h"
#include "Core/Singleton.h"
#include "Core/ISysytem.h"
#include "Core/ReferencePtr.h"

#include "Asset/AssetInfo.h"
#include "Asset/Asset.h"
#include "Asset/AssetPackage/IAssetPackage.h"
#include "Core/GUID.h"

#include "Threading/SpinLock.h"

#include <unordered_map>
#include <mutex>

namespace Insight
{
    class IObject;

    namespace Runtime
    {
        class AssetUser;
        class IAssetPackage;
        class IAssetImporter;

        /// @brief Manage all references to known assets on disk.
        class IS_RUNTIME AssetRegistry : public Core::Singleton<AssetRegistry>, public Core::ISystem
        {
        public:
            AssetRegistry() = default;
            virtual ~AssetRegistry() override = default;

            // Begin - ISystem -
            IS_SYSTEM(AssetRegistry);
            virtual void Initialise() override;
            virtual void Shutdown() override;
            // End - ISystem -

            IAssetPackage* CreateAssetPackage(std::string_view name);

            IAssetPackage* LoadAssetPackage(std::string_view path);
            void UnloadAssetPackage(std::string_view path);

            /// @brief 
            /// @param metaFileDirectory 
            /// @param assetReativeBaseDirectory 
            void SetDebugDirectories(std::string metaFileDirectory, std::string assetReativeBaseDirectory);

            const AssetInfo* AddAssetFromPackage(std::string_view path, IAssetPackage* package);
            const AssetInfo* AddAssetFromDisk(std::string_view path, IAssetPackage* package);
            const AssetInfo* AddAsset(std::string_view path, IAssetPackage* package, bool enableMetaFile, bool checkOnDisk);
            void RemoveAsset(std::string_view path);

            void UpdateMetaData(AssetInfo* assetInfo, AssetUser* object);
            void UpdateMetaData(AssetUser* object);
            void DeserialiseAssetUser(AssetInfo* assetInfo, AssetUser* object) const;

            Ref<Asset> LoadAsset2(std::string path);
            const AssetInfo* GetAssetInfo(const std::string& path) const;
            std::vector<Byte> LoadAssetData(std::string_view path) const;


            DEPRECATED_MSG("Use 'Ref<Asset> LoadAsset'.")
            std::vector<Byte> LoadAsset(std::string_view path) const;

            Ref<Asset> GetAsset2(const std::string& path) const;
            DEPRECATED_MSG("Use 'Ref<Asset> GetAsset'.")
            const AssetInfo* GetAsset(const Core::GUID& guid) const;
            DEPRECATED_MSG("Use 'Ref<Asset> GetAsset'.")
            const AssetInfo* GetAsset(std::string_view path) const;

            std::vector<const AssetInfo*> GetAllAssetInfos() const;
            std::vector<const AssetInfo*> GetAllAssetsWithExtension(std::string_view extension) const;
            std::vector<const AssetInfo*> GetAllAssetsWithExtensions(std::vector<std::string_view> extensions) const;

            IAssetPackage* GetAssetPackageFromPath(std::string_view path) const;
            IAssetPackage* GetAssetPackageFromName(std::string_view name) const;
            std::vector<IAssetPackage*> GetAllAssetPackages() const;

            IAssetPackage* GetAssetPackageFromAsset(const AssetInfo* assetInfo) const;

            /// @brief Add all asset within a folder. 
            void AddAssetsInFolder(std::string_view path, IAssetPackage* package);
            /// @brief Add all asset within a folder, recursive.
            void AddAssetsInFolder(std::string_view path, IAssetPackage* package, bool recursive);
            /// @brief Add all asset within a folder, recursive, with the option of disabling meta files (should be false for engine/editor folders).
            void AddAssetsInFolder(std::string_view path, IAssetPackage* package, bool recursive, bool enableMetaFiles);

            /// @brief Register an IObject to an AssetInfo allowing for the object to be retevied later 
            /// from the AssetInfo pointer.
            void RegisterObjectToAsset(const AssetInfo* assetInfo, IObject* object);
            void UnregisterObjectToAsset(const IObject* object);

            std::vector<IObject*> GetObjectsFromAsset(const Core::GUID& guid) const;

        private:
            IAssetPackage* CreateAssetPackageInternal(std::string_view name, std::string_view path, AssetPackageType packageType);

            bool HasAssetFromGuid(const Core::GUID& guid) const;
            bool HasAssetFromPath(std::string_view path) const;

            void LoadMetaData(AssetInfo* assetInfo);

            bool AssetInfoValidate(const AssetInfo* assetInfo) const;

            std::string ValidatePath(const std::string& path) const;

        private:
            /// @brief Store all asset infos here, then give pointers to the packages which they are included in.
            /// This unordered_map acts as the owner all of the pointers.
            std::unordered_map<std::string, AssetInfo*> m_pathToAssetInfo;

            std::vector<IAssetPackage*> m_assetPackages;

            std::vector<IAssetImporter*> m_importers;
            Threading::SpinLock m_loadedAssetLock;
            std::unordered_map<std::string, Ref<Asset>> m_loadedAssets;

            std::unordered_map<Core::GUID, std::unordered_set<const IObject*>> m_assetToObjects;
            mutable std::mutex m_assetToObjectGuid;

            //std::unordered_map<Core::GUID, AssetInfo*> m_guidToAssetInfoLookup;
            //std::unordered_map<std::string, Core::GUID> m_pathToGuidLookup;

            std::string m_debugMetaFileDirectory;
            std::string m_assetReativeBaseDirectory;
        };
    }
}