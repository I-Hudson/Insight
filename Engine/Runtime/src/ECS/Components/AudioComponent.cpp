#include "ECS/Components/AudioComponent.h"

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
	}
}