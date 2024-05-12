#pragma once

#include "Asset/Importers/IAssetImporter.h"

namespace Insight
{
    namespace Runtime
    {
        class ShaderImporter : public IAssetImporter
        {
        public:
            ShaderImporter();
            virtual ~ShaderImporter() override;

            virtual Ref<Asset> CreateAsset(const AssetInfo* assetInfo) const override;
            virtual void Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const override;
        };
    }
}