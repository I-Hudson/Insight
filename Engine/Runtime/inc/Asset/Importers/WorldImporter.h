#pragma once

#include "Asset/Importers/IAssetImporter.h"

namespace Insight
{
    namespace Runtime
    {
        /// @brief Special importer which is not used within the Asset system but used sirectly from 'WorldSystem'.
        class WorldImporter : public IAssetImporter
        {
        public:
            WorldImporter();
            virtual ~WorldImporter() override;

            virtual Ref<Asset> CreateAsset(const AssetInfo* assetInfo) const override;
            virtual void Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const override;
            virtual Reflect::Type GetAssetType() const override;
            virtual bool AllowAssetImportingFromAssetRegistry() const override { return false; }
        };
    }
}