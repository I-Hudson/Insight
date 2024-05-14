#pragma once

#if defined(IS_PLATFORM_WINDOWS) && defined(IS_AUDIO_XAUDIO_ENABLE)

#include "Audio/Backend/IAudioBackend.h"

#include <xaudio2.h>
#undef PlaySound

#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>

#include <thread>
#include <vector>

namespace Insight
{
    namespace Runtime
    {
        class AssetInfo;

        struct XAudioSound
        {
            IXAudio2SourceVoice* SourceVoice = nullptr;
            BYTE* Buffer = nullptr;
            u32 BufferSize = 0;
            u32 Id = 0;
            const AssetInfo* AssetInfo;
            bool IsFree = true;
        };

        class XAudio2Backend : public IAudioBackend
        {
        public:
            XAudio2Backend();
            virtual ~XAudio2Backend() override;

            virtual u32 PlaySound(const char* fileName) override;
            virtual void StopSound(const u32 soundId) override;

        private:
            bool CreateMFReader(SourceReaderContext& renderContext, WAVEFORMATEX& wfx);
            bool FindChunk(const std::vector<u8>& fileBuffer, const DWORD fourcc, DWORD& chunkSize, DWORD& chunkDataPosition) const;
            bool ReadChunkData(const std::vector<u8>& fileBuffer, void* buffer, const DWORD bufferSize, const DWORD bufferOffset) const;

            void AudioThreadUpdates(XAudio2Backend* audioBackend);

        private:
            IXAudio2* m_audio2 = nullptr;
            IXAudio2MasteringVoice* m_masteringVoice = nullptr;

            std::atomic<bool> m_stopAudioThread = false;
            std::thread m_audioThread;
            std::vector<XAudioSound> m_playingSounds;
        };
    }
}
#endif