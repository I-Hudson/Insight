#pragma once

#include "Runtime/Defines.h"

#include "ECS/Entity.h"

#include "Maths/Vector3.h"
#include "Generated/LightComponent_reflect_generated.h"

namespace Insight
{
	namespace ECS
	{
		REFLECT_CLASS(IHT_ABSTRACT_COMPONENT)
		class IS_RUNTIME LightComponent : public Component
		{
			REFLECT_GENERATED_BODY()
		public:
			IS_COMPONENT(LightComponent);

			LightComponent();
			virtual ~LightComponent() override;

			IS_SERIALISABLE_H(LightComponent);

			Maths::Vector3 GetLightColour() const;
			float GetIntensity() const;

		private:
			REFLECT_PROPERTY(EditorVisible, Meta(ClampMin(0.0), ClampMax(1.0)))
			Maths::Vector3 m_lightColour = Maths::Vector3::One;
			REFLECT_PROPERTY(EditorVisible)
			float m_intensity = 1.0f;
		};
	}

	OBJECT_SERIALISER(ECS::LightComponent, 1,
		SERIALISE_PROPERTY(Maths::Vector3, m_lightColour, 1, 0)
		SERIALISE_PROPERTY(float, m_intensity, 1, 0)
		SERIALISE_BASE(ECS::Component, 1, 0)
	);
}