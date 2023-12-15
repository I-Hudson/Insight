#pragma once

#include "Asset/AssetPackage/IAssetPackage.h"

namespace Insight
{
    namespace Runtime
    {
        /// @brief Contain information about all assets with this package and how to load them.
        class IS_RUNTIME AssetPackageFileSystem : public IAssetPackage
        {
        public:
            AssetPackageFileSystem() = delete;
            AssetPackageFileSystem(std::string_view packagePath, std::string_view packageName);
            AssetPackageFileSystem(const AssetPackageFileSystem& other) = delete;
            virtual ~AssetPackageFileSystem() override;

            IS_SERIALISABLE_H(AssetPackageFileSystem)

        private:
            virtual std::vector<Byte> LoadInteral(const AssetInfo* assetInfo) const override;
        };
    }
}