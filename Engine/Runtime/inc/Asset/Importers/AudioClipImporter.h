#pragma once

#include "Asset/Importers/IAssetImporter.h"

namespace Insight
{
    namespace Runtime
    {
        class AudioClipImporter : public IAssetImporter
        {
        public:
            AudioClipImporter();
            virtual ~AudioClipImporter() override;

            virtual Ref<Asset> CreateAsset(const AssetInfo* assetInfo) const override;
            virtual void Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const override;
        };
    }
}