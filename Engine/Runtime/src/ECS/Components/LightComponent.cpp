#include "ECS/Components/LightComponent.h"

namespace Insight
{
	namespace ECS
	{
		LightComponent::LightComponent()
		{
		}

		LightComponent::~LightComponent()
		{ }

		IS_SERIALISABLE_CPP(LightComponent)

		Maths::Vector3 LightComponent::GetLightColour() const
		{
			return m_lightColour;
		}

		float LightComponent::GetIntensity() const
		{
			return m_intensity;
		}

	}
}