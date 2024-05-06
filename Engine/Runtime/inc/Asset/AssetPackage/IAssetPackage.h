#pragma once

#include "Core/IObject.h"
#include "Runtime/Defines.h"

namespace Insight
{
    namespace Runtime
    {
        class AssetInfo;

        enum class AssetPackageType
        {
            FileSystem,
            Zip
        };

        /// @brief Contain information about all assets with this package and how to load them.
        class IS_RUNTIME IAssetPackage : public IObject
        {
        public:
            IAssetPackage() = delete;
            IAssetPackage(std::string_view packagePath, std::string_view packageName, AssetPackageType packageType);
            IAssetPackage(const IAssetPackage& other) = delete;
            virtual ~IAssetPackage();

            constexpr static const char* c_FileExtension = ".isassetpackage";

            std::string_view GetPath() const;
            std::string_view GetName() const;
            AssetPackageType GetPackageType() const;

            DEPRECATED_MSG("Use AddAsset with AssetInfo augment")
            const AssetInfo* AddAsset(std::string_view path);
            const AssetInfo* AddAsset(AssetInfo* assetInfo);

            DEPRECATED_MSG("Use RemoveAsset with AssetInfo augment")
            void RemoveAsset(std::string_view path);
            DEPRECATED_MSG("Use RemoveAsset with AssetInfo augment")
            void RemoveAsset(const Core::GUID& guid);
            void RemoveAsset(AssetInfo* assetInfo);

            bool HasAsset(std::string_view path) const;
            bool HasAsset(const Core::GUID& guid) const;
            bool HasAsset(const AssetInfo* assetInfo) const;


            const AssetInfo* GetAsset(std::string_view path) const;
            const AssetInfo* GetAsset(const Core::GUID& guid) const;
            const AssetInfo* GetAsset(const AssetInfo* assetInfo) const;

            /// @brief Replace an asset within the package with another asset,
            /// @param oldAsset 
            /// @param newAsset 
            void ReplaceAsset(AssetInfo* oldAsset, AssetInfo* newAsset);

            const std::vector<AssetInfo*>& GetAllAssetInfos() const;

            std::vector<Byte> LoadAsset(std::string_view path) const;
            std::vector<Byte> LoadAsset(Core::GUID guid) const;

            virtual void BuildPackage(std::string_view path);

            void Destroy();

        protected:
            virtual std::vector<Byte> LoadInteral(const AssetInfo* assetInfo) const = 0;

            Core::GUID GetGuidFromPath(std::string_view path) const;

            void LoadMetaData(AssetInfo* assetInfo);
            bool AssetInfoValidate(const AssetInfo* assetInfo) const;

        protected:
            std::string m_packagePath;
            std::string m_packageName;
            AssetPackageType m_packageType;

            mutable std::mutex m_packageLock;

            std::vector<AssetInfo*> m_assetInfos;
            std::unordered_map<std::string, AssetInfo*> m_assetInfosFromPath;
            std::unordered_map<Core::GUID, AssetInfo*> m_assetInfosFromGuid;
        };
    }
}