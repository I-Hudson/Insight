#pragma once

#include "Runtime/Defines.h"
#include "Core/Singleton.h"
#include "Core/ISysytem.h"

#include "Asset/AssetInfo.h"
#include "Core/GUID.h"

#include <unordered_map>

namespace Insight::Runtime
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

        const AssetInfo* GetAsset(const Core::GUID& guid) const;
        const AssetInfo* GetAsset(std::string_view path) const;

        /// @brief Add all asset within a folder. 
        void AddAssetsInFolder(std::string_view path, AssetPackage* package);
        /// @brief Add all asset within a folder, recursive.
        void AddAssetsInFolder(std::string_view path, AssetPackage* package, bool recursive);
        /// @brief Add all asset within a folder, recursive, with the option of disabling meta files (should be false for engine/editor folders).
        void AddAssetsInFolder(std::string_view path, AssetPackage* package, bool recursive, bool enableMetaFiles);

    private:
        bool HasAssetFromGuid(const Core::GUID& guid) const;
        bool HasAssetFromPath(std::string_view path) const;

        void LoadMetaData(AssetInfo* assetInfo);

        bool AssetInfoValidate(const AssetInfo* assetInfo) const;

    private:
        std::vector<AssetPackage*> m_assetPackages;
        //std::unordered_map<Core::GUID, AssetInfo*> m_guidToAssetInfoLookup;
        //std::unordered_map<std::string, Core::GUID> m_pathToGuidLookup;

        std::string m_debugMetaFileDirectory;
        std::string m_assetReativeBaseDirectory;
    };
}