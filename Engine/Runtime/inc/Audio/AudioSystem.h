#pragma once

#include "Runtime/Defines.h"
#include "Core/ISysytem.h"
#include "Core/Singleton.h"
#include "Core/ReferencePtr.h"

#include "Asset/Assets/AudioClip.h"

#include <vector>

namespace Insight
{
    namespace Runtime
    {
        class IAudioBackend;

        class IS_RUNTIME AudioSystem : public Core::ISystem
        {
        public:
            AudioSystem();
            virtual ~AudioSystem() override;

            IS_SYSTEM(AudioSystem);

            virtual void Initialise() override;
            virtual void Shutdown() override;

            static u32 PlaySound(const char* fileName);
            static u32 PlaySound(Ref<Runtime::AudioClipAsset> audioClip);

            static void StopSound(const u32 soundId);

            static void SetVolume(const u32 soundId, float volume);

        private:
            static IAudioBackend* m_audioBackend;
        };
    }
}