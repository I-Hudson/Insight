#pragma once

#include "Runtime/Defines.h"

#include "ECS/Components/LightComponent.h"

#include "Generated/SpotLightComponent_reflect_generated.h"

namespace Insight
{
	namespace ECS
	{
		REFLECT_CLASS()
		class IS_RUNTIME SpotLightComponent : public LightComponent
		{
			REFLECT_GENERATED_BODY()
		public:
			IS_COMPONENT(SpotLightComponent);

			SpotLightComponent();
			virtual ~SpotLightComponent() override;

			IS_SERIALISABLE_H(SpotLightComponent)

		private:
			REFLECT_PROPERTY(EditorVisible)
			float m_radius = 5.0f;
		};
	}

	OBJECT_SERIALISER(ECS::SpotLightComponent, 1,
		SERIALISE_PROPERTY(float, m_radius, 1, 0)
		SERIALISE_BASE(ECS::LightComponent, 1, 0)
	);
}