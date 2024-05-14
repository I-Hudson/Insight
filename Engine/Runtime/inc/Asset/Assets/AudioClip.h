#pragma once

#include "Asset/Asset.h"

#include <Reflect/Core/Defines.h>

#include "Generated/AudioClip_reflect_generated.h"

namespace Insight
{
    namespace Runtime
    {
        class AudioClipImporter;

        REFLECT_CLASS();
        class AudioClipAsset : public Asset
        {
            REFLECT_GENERATED_BODY();
        public:
            AudioClipAsset(const AssetInfo* assetInfo);
            virtual ~AudioClipAsset() override;

            // Asset - Begin
        protected:
            virtual void OnUnload() override;
            // Asset - End

        private:
            friend AudioClipImporter;
        };
    }
}