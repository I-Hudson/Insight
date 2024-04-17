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

			IS_SERIALISABLE_H(TagComponent)

		private:
			Maths::Vector3 m_lightColour = Maths::Vector3::Zero;
			float m_intensity = 0.0f;
		};
	}

	OBJECT_SERIALISER(ECS::LightComponent, 1,
		SERIALISE_PROPERTY(Maths::Vector3, m_lightColour, 1, 0)
		SERIALISE_PROPERTY(float, m_intensity, 1, 0)
		SERIALISE_BASE(ECS::Component, 2, 0)
	);
}