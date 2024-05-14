#include "Audio/AudioSystem.h"

#include "Audio/Backend/XAudio2Backend.h"
#include "Audio/Backend/MiniAudioBackend.h"

#include "Core/Memory.h"

namespace Insight
{
    namespace Runtime
    {
        IAudioBackend* AudioSystem::m_audioBackend = nullptr;

        AudioSystem::AudioSystem()
        { 

        }
        AudioSystem::~AudioSystem()
        { 
        }

        void AudioSystem::Initialise()
        {
#if defined(IS_PLATFORM_WINDOWS) && defined(IS_AUDIO_XAUDIO_ENABLE)
            m_audioBackend = ::New<XAudio2Backend>();
#elif defined(IS_AUDIO_MINIAUDIO_ENABLE)
            m_audioBackend = ::New<MiniAudioBackend>();
#endif
            Core::ISystem::Initialise();
        }

        void AudioSystem::Shutdown()
        {
            ::Delete(m_audioBackend);

            Core::ISystem::Shutdown();
        }

        u32 AudioSystem::PlaySound(const char* fileName)
        {
            return m_audioBackend->PlaySound(fileName);
        }

        void AudioSystem::StopSound(const u32 soundId)
        {
            m_audioBackend->StopSound(soundId);
        }
    }
}