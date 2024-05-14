#include "Asset/Importers/AudioClipImporter.h"
#include "Asset/Assets/AudioClip.h"

#include "Graphics/RenderContext.h"

namespace Insight
{
    namespace Runtime
    {
        AudioClipImporter::AudioClipImporter()
            : IAssetImporter({ ".mp3", ".wav" })
        { }

        AudioClipImporter::~AudioClipImporter()
        { }

        Ref<Asset> AudioClipImporter::CreateAsset(const AssetInfo* assetInfo) const
        {
            return Ref<Asset>(::New<AudioClipAsset>(assetInfo));
        }

        void AudioClipImporter::Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const
        {
            Ref<AudioClipAsset> audioAsset = asset.As<AudioClipAsset>();
            audioAsset->m_assetState = AssetState::Loaded;
        }
    }
}