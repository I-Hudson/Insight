#include "ECS/Components/AudioComponent.h"

#include "Audio/AudioSystem.h"

namespace Insight
{
	namespace ECS
	{
		AudioComponent::AudioComponent()
		{
		}

		AudioComponent::~AudioComponent()
		{
		}

		IS_SERIALISABLE_CPP(AudioComponent);

		Ref<Runtime::AudioClipAsset> AudioComponent::GetAudioClip() const
		{
			return m_audioClip;
		}

		void AudioComponent::SetAudioClip(Ref<Runtime::AudioClipAsset> audioClip)
		{
			m_audioClip = audioClip;
		}

		void AudioComponent::Play()
		{
			m_audioClipId = Runtime::AudioSystem::PlaySound(m_audioClip);
		}

		void AudioComponent::Stop()
		{
			m_audioClipId = 0;
			Runtime::AudioSystem::StopSound(m_audioClip);
		}

		float AudioComponent::GetVolume() const
		{
			return m_volume;
		}

		void AudioComponent::SetVolume(const float volume)
		{
			if (m_volume != volume)
			{
				m_volume = volume;
				Runtime::AudioSystem::SetVolume(m_audioClipId, m_volume);
			}
		}
	}
}