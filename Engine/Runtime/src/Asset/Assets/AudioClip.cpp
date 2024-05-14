#include "Asset/Assets/AudioClip.h"

namespace Insight
{
    namespace Runtime
    {
        AudioClipAsset::AudioClipAsset(const AssetInfo* assetInfo)
            : Asset(assetInfo)
        { }

        AudioClipAsset::~AudioClipAsset()
        { }

        void AudioClipAsset::OnUnload()
        {

        }
    }
}