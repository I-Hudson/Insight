#pragma once

#include "Runtime/Defines.h"

#include "ECS/Entity.h"

#include "Core/ReferencePtr.h"
#include "Asset/Assets/AudioClip.h"

#include "Generated/AudioComponent_reflect_generated.h"

namespace Insight
{
	namespace ECS
	{
		REFLECT_CLASS();
		class IS_RUNTIME AudioComponent : public Component
		{
			REFLECT_GENERATED_BODY();
		public:
			IS_COMPONENT(AudioComponent);

			AudioComponent();
			virtual ~AudioComponent() override;

			IS_SERIALISABLE_H(AudioComponent);

			Ref<Runtime::AudioClipAsset> GetAudioClip() const;
			void SetAudioClip(Ref<Runtime::AudioClipAsset> audioClip);

		private:
			REFLECT_PROPERTY(EditorVisible);
			Ref<Runtime::AudioClipAsset> m_audioClip;
			REFLECT_PROPERTY(EditorVisible, Meta(ClampMin(0.0), ClampMax(1.0)));
			float m_volume = 0.5f;
			REFLECT_PROPERTY(EditorVisible, Meta(ClampMin(0.0), ClampMax(1.0)));
			float m_pitch = 0.5f;
		};
	}
	OBJECT_SERIALISER(ECS::AudioComponent, 1,
		SERIALISE_BASE(ECS::Component, 1, 0)
		//SERIALISE_PROPERTY(Ref<Runtime::Asset>, m_audioClip, 1, 0)
		SERIALISE_PROPERTY(float, m_volume, 1, 0)
		SERIALISE_PROPERTY(float, m_pitch, 1, 0)
	);
}