#include "Asset/Importers/WorldImporter.h"
#include "World/World.h"

#include "Asset/AssetRegistry.h"
#include "Serialisation/Serialisers/BinarySerialiser.h"

namespace Insight
{
    namespace Runtime
    {
        WorldImporter::WorldImporter()
            : IAssetImporter({ World::c_FileExtension })
        {
        }

        WorldImporter::~WorldImporter()
        {
        }

        Ref<Asset> WorldImporter::CreateAsset(const AssetInfo* assetInfo) const
        {
            return Ref<Asset>(::New<World>(assetInfo));
        }

        void WorldImporter::Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const
        {
            Ref<World> worldAsset = asset.As<World>();

            std::vector<Byte> worldData = AssetRegistry::Instance().LoadAssetData(path);
            if (worldData.empty())
            {
                return;
            }

            Serialisation::BinarySerialiser serialiser(true);
            if (!serialiser.Deserialise(worldData))
            {
                return;
            }

            worldAsset->Deserialise(&serialiser);

            worldAsset->m_assetState = AssetState::Loaded;
        }

        Reflect::Type WorldImporter::GetAssetType() const
        {
            return World::GetStaticTypeInfo().GetType();
        }
    }
}