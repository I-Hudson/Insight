#pragma once

#include "Asset/Importers/IAssetImporter.h"

namespace Insight
{
    namespace Runtime
    {
        class TextureImporter : public IAssetImporter
        {
        public:
            TextureImporter();

            virtual Ref<Asset> Import(const AssetInfo* assetInfo, const std::string_view path) const override;

        private:

        };
    }
}