#pragma once
#if defined(IS_PLATFORM_WINDOWS) && defined(IS_AUDIO_MINIAUDIO_ENABLE)
#include "Audio/Backend/IAudioBackend.h"

#include <miniaudio.h>
#undef PlaySound

#include <vector>
#include <mutex>

namespace Insight
{
    namespace Runtime
    {
        class AssetInfo;

        struct MiniAudioSound
        {
            ma_sound AudioSound;
            ma_decoder AudioDecoder;
            std::vector<u8> SoundBuffer;
            const AssetInfo* AssetInfo;
            u32 Id = 0;
            bool InUse = true;

            void Release();
        };

        class MiniAudioBackend : public IAudioBackend
        {
        public:
            MiniAudioBackend();
            virtual ~MiniAudioBackend() override;

            virtual u32 PlaySound(const char* fileName) override;
            virtual void StopSound(const u32 soundId) override;

        private:
            ma_engine m_audioEngine;
            std::vector<MiniAudioSound*> m_sounds;
            std::mutex m_soundsLock;
        };
    }
}
#endif