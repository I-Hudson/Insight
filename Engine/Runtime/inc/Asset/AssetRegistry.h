#pragma once

#include "Runtime/Defines.h"
#include "Core/Singleton.h"
#include "Core/ISysytem.h"

#include "Asset/AssetInfo.h"
#include "Core/GUID.h"

#include <unordered_map>
#include <mutex>

namespace Insight
{
    class IObject;

    namespace Runtime
    {
        class AssetUser;
        class AssetPackage;

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

            AssetPackage* CreateAssetPackage(std::string_view name);

            AssetPackage* LoadAssetPackage(std::string_view path);
            void UnloadAssetPackage(std::string_view path);

            /// @brief 
            /// @param metaFileDirectory 
            /// @param assetReativeBaseDirectory 
            void SetDebugDirectories(std::string metaFileDirectory, std::string assetReativeBaseDirectory);

            const AssetInfo* AddAsset(std::string_view path, AssetPackage* package);
            const AssetInfo* AddAsset(std::string_view path, AssetPackage* package, bool enableMetaFile);
            void RemoveAsset(std::string_view path, AssetPackage* package);

            void UpdateMetaData(AssetInfo* assetInfo, AssetUser* object);
            void UpdateMetaData(AssetUser* object);
            void DeserialiseAssetUser(AssetInfo* assetInfo, AssetUser* object) const;

            std::vector<Byte> LoadAsset(std::string_view path) const;

            const AssetInfo* GetAsset(const Core::GUID& guid) const;
            const AssetInfo* GetAsset(std::string_view path) const;

            std::vector<const AssetInfo*> GetAllAssetInfos() const;
            std::vector<const AssetInfo*> GetAllAssetsWithExtension(std::string_view extension) const;
            std::vector<const AssetInfo*> GetAllAssetsWithExtensions(std::vector<std::string_view> extensions) const;

            AssetPackage* GetAssetPackageFromPath(std::string_view path) const;
            AssetPackage* GetAssetPackageFromName(std::string_view name) const;
            std::vector<AssetPackage*> GetAllAssetPackages() const;

            /// @brief Add all asset within a folder. 
            void AddAssetsInFolder(std::string_view path, AssetPackage* package);
            /// @brief Add all asset within a folder, recursive.
            void AddAssetsInFolder(std::string_view path, AssetPackage* package, bool recursive);
            /// @brief Add all asset within a folder, recursive, with the option of disabling meta files (should be false for engine/editor folders).
            void AddAssetsInFolder(std::string_view path, AssetPackage* package, bool recursive, bool enableMetaFiles);

            /// @brief Register an IObject to an AssetInfo allowing for the object to be retevied later 
            /// from the AssetInfo pointer.
            void RegisterObjectToAsset(const AssetInfo* assetInfo, IObject* object);
            void UnregisterObjectToAsset(const IObject* object);

            IObject* GetObjectFromAsset(const Core::GUID& guid);

        private:
            AssetPackage* CreateAssetPackageInternal(std::string_view name, std::string_view path);

            bool HasAssetFromGuid(const Core::GUID& guid) const;
            bool HasAssetFromPath(std::string_view path) const;

            void LoadMetaData(AssetInfo* assetInfo);

            bool AssetInfoValidate(const AssetInfo* assetInfo) const;

        private:
            std::vector<AssetPackage*> m_assetPackages;

            std::unordered_map<Core::GUID, IObject*> m_assetToObject;
            mutable std::mutex m_assetToObjectGuid;

            //std::unordered_map<Core::GUID, AssetInfo*> m_guidToAssetInfoLookup;
            //std::unordered_map<std::string, Core::GUID> m_pathToGuidLookup;

            std::string m_debugMetaFileDirectory;
            std::string m_assetReativeBaseDirectory;
        };
    }
}