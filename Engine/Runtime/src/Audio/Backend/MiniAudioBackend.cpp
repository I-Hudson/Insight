#if defined(IS_PLATFORM_WINDOWS) && defined(IS_AUDIO_MINIAUDIO_ENABLE)
#include "Audio/Backend/MiniAudioBackend.h"

#include "Core/Asserts.h"
#include "Asset/AssetRegistry.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#undef PlaySound

namespace Insight
{
    namespace Runtime
    {
        void MiniAudioSound::Release()
        {
            if (InUse)
            {
                ASSERT(ma_sound_stop(&AudioSound) == MA_SUCCESS);
                ma_sound_uninit(&AudioSound);
                ASSERT(ma_decoder_uninit(&AudioDecoder) == MA_SUCCESS);
            }
        }

        MiniAudioBackend::MiniAudioBackend()
        {
            ASSERT(ma_engine_init(NULL, &m_audioEngine) == MA_SUCCESS);
        }

        MiniAudioBackend::~MiniAudioBackend()
        {
            std::lock_guard lock(m_soundsLock);
            for (size_t i = 0; i < m_sounds.size(); ++i)
            {
                m_sounds[i]->Release();
                ::Delete(m_sounds[i]);
            }

            ma_engine_uninit(&m_audioEngine);
        }

        u32 MiniAudioBackend::PlaySound(const char* fileName)
        {
            const std::string audioFilePath = AssetRegistry::Instance().ValidatePath(fileName);

            const AssetInfo* assetInfo = AssetRegistry::Instance().GetAssetInfo(audioFilePath);
            MiniAudioSound* miniAudioSound = nullptr;
            {
                std::lock_guard lock(m_soundsLock);
                for (size_t i = 0; i < m_sounds.size(); ++i)
                {
                    MiniAudioSound* sound = m_sounds[i];
                    if (sound->InUse && sound->AssetInfo == assetInfo)
                    {
                        return sound->Id;
                    }
                    else if (!sound->InUse && !miniAudioSound)
                    {
                        miniAudioSound = sound;
                    }
                }
            }

            std::vector<u8> fileBuffer = AssetRegistry::Instance().LoadAssetData(audioFilePath);
            if (fileBuffer.empty())
            {
                return 0;
            }

            if (!miniAudioSound)
            {
                miniAudioSound = ::New<MiniAudioSound>();
                miniAudioSound->Id = m_sounds.size() + 1;
                miniAudioSound->SoundBuffer = std::move(fileBuffer);
                miniAudioSound->AssetInfo = assetInfo;
                miniAudioSound->InUse = true;

                std::lock_guard lock(m_soundsLock);
                m_sounds.push_back(miniAudioSound);
            }
            else
            {
                miniAudioSound->SoundBuffer = std::move(fileBuffer);
                miniAudioSound->AssetInfo = assetInfo;
                miniAudioSound->InUse = true;
            }

            const ma_result decoderInitResult = ma_decoder_init_memory(miniAudioSound->SoundBuffer.data(), miniAudioSound->SoundBuffer.size(), NULL, &miniAudioSound->AudioDecoder);
            if (decoderInitResult != MA_SUCCESS)
            {
                return 0;
            }

            const ma_result soundInitResult = ma_sound_init_from_data_source(&m_audioEngine, &miniAudioSound->AudioDecoder, 0 , NULL, &miniAudioSound->AudioSound);
            if (soundInitResult != MA_SUCCESS)
            {
                return 0;
            }

            const ma_result soundStartResult = ma_sound_start(&miniAudioSound->AudioSound);
            if (soundStartResult != MA_SUCCESS)
            {
                return 0;
            }

            return miniAudioSound->Id;
        }

        u32 MiniAudioBackend::PlaySound(Ref<Runtime::AudioClipAsset> audioClip)
        {
            ASSERT(audioClip);
            return PlaySound(audioClip->GetAssetInfo()->GetFullFilePath().data());
        }

        void MiniAudioBackend::StopSound(const u32 soundId)
        {
            std::lock_guard lock(m_soundsLock);
            for (size_t i = 0; i < m_sounds.size(); ++i)
            {
                MiniAudioSound* sound = m_sounds[i];
                if (sound->Id == soundId && sound->InUse)
                {
                    sound->Release();

                    sound->SoundBuffer.resize(0);
                    sound->AssetInfo = nullptr;
                    sound->InUse = false;

                    return;
                }
            }
        }

        void MiniAudioBackend::StopSound(Ref<Runtime::AudioClipAsset> audioClip)
        {
            ASSERT(audioClip);
            u32 soundId = 0;
            {
                std::lock_guard lock(m_soundsLock);
                for (size_t i = 0; i < m_sounds.size(); ++i)
                {
                    if (m_sounds[i]->AssetInfo == audioClip->GetAssetInfo())
                    {
                        soundId = m_sounds[i]->Id;
                        break;
                    }
                }
            }

            if (soundId != 0)
            {
                StopSound(soundId);
            }
        }

        void MiniAudioBackend::SetVolume(const u32 soundId, float volume)
        {
            std::lock_guard lock(m_soundsLock);
            for (size_t i = 0; i < m_sounds.size(); ++i)
            {
                MiniAudioSound* sound = m_sounds[i];
                if (sound->Id == soundId && sound->InUse)
                {
                    ma_sound_set_volume(&sound->AudioSound, volume);
                    return;
                }
            }
        }
    }
}
#endif