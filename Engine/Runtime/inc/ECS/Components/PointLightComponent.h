#pragma once

#include "Runtime/Defines.h"

#include "ECS/Components/LightComponent.h"

#include "Generated/PointLightComponent_reflect_generated.h"

namespace Insight
{
	namespace ECS
	{
		REFLECT_CLASS()
		class IS_RUNTIME PointLightComponent : public LightComponent
		{
			REFLECT_GENERATED_BODY()
		public:
			IS_COMPONENT(PointLightComponent);

			PointLightComponent();
			virtual ~PointLightComponent() override;

			IS_SERIALISABLE_H(PointLightComponent);

			float GetRadius() const;

		private:
			REFLECT_PROPERTY(EditorVisible)
			float m_radius = 5.0f;
		};
	}

	OBJECT_SERIALISER(ECS::PointLightComponent, 1,
		SERIALISE_PROPERTY(float, m_radius, 1, 0)
		SERIALISE_BASE(ECS::LightComponent, 1, 0)
	);
}