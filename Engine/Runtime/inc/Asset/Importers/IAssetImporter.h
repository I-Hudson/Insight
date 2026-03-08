#pragma once

#include "Core/ReferencePtr.h"
#include "Asset/Asset.h"

namespace Insight
{
    namespace Runtime
    {
        class IAssetImporter
        {
        public:
            IAssetImporter() = delete;
            IAssetImporter(std::vector<const char*> validFileExtensions);
            virtual ~IAssetImporter();

            bool IsValidImporterForFileExtension(const char* fileExtension) const;

            virtual Ref<Asset> CreateAsset(const AssetInfo* assetInfo) const = 0;
            virtual void Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const = 0;
            /// @brief 'ImportAndConvertToEngineFormat' should return an Asset which has all possible data available.
            virtual void ImportAndConvertToEngineFormat(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const;
            virtual bool CanConvertToEngineFormat() const { return false; }
            virtual Reflect::Type GetAssetType() const = 0;
            /// @brief Define should this importer be allowed to be used via the AssetRegistry our must be made in place and 'Import' be called
            /// by a local stack instance.
            /// @return bool
            virtual bool AllowAssetImportingFromAssetRegistry() const { return true; }

        private:
            std::vector<const char*> m_validFileExtensions;
        };
    }
}