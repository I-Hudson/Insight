#include "ECS/Components/PointLightComponent.h"

namespace Insight
{
	namespace ECS
	{
		PointLightComponent::PointLightComponent()
		{
		}

		PointLightComponent::~PointLightComponent()
		{ }

		IS_SERIALISABLE_CPP(PointLightComponent)

		float PointLightComponent::GetRadius() const
		{
			return m_radius;
		}

	}
}