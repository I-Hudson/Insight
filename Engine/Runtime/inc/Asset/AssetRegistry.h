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

        const AssetInfo* AddAsset(std::string_view path);
        void RemoveAsset(std::string_view path);

        void UpdateMetaData(AssetUser* object);

        const AssetInfo* GetAsset(const Core::GUID& guid) const;
        const AssetInfo* GetAsset(std::string_view path) const;

        /// @brief Add all asset within a folder. 
        void AddAssetsInFolder(std::string_view path);
        /// @brief Add all asset within a folder, recursive. 
        void AddAssetsInFolder(std::string_view path, bool recursive);

    private:
        bool HasAssetFromGuid(const Core::GUID& guid) const;
        bool HasAssetFromPath(std::string_view path) const;

        void LoadMetaData(AssetInfo* assetInfo);

    private:
        std::unordered_map<Core::GUID, AssetInfo*> m_guidToAssetInfoLookup;
        std::unordered_map<std::string, Core::GUID> m_pathToGuidLookup;
    };
}