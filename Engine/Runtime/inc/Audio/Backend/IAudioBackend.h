#pragma once

#include "Core/TypeAlias.h"

#include "Core/ReferencePtr.h"
#include "Asset/Assets/AudioClip.h"

namespace Insight
{
    namespace Runtime
    {
        class IAudioBackend
        {
        public:
            virtual ~IAudioBackend() { }

            virtual u32 PlaySound(const char* fileName) = 0;
            virtual u32 PlaySound(Ref<Runtime::AudioClipAsset> audioClip) = 0;
            
            virtual void StopSound(const u32 soundId) = 0;
            virtual void StopSound(Ref<Runtime::AudioClipAsset> audioClip) = 0;
        };
    }
}