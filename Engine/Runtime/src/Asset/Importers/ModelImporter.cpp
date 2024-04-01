#include "Asset/Importers/ModelImporter.h"

#include "Asset/Assets/Model.h"

namespace Insight
{
    namespace Runtime
    {
        ModelImporter::ModelImporter()
            : IAssetImporter(
                {
                    ".obj",
                    ".fbx",
                    ".gltf",
                })
        {
        }

        Ref<Asset> ModelImporter::Import(const AssetInfo* assetInfo, const std::vector<u8>& data) const
        {
            Ref<ModelAsset> modelAsset = New<ModelAsset>(assetInfo);
            return modelAsset;
        }
    }
}