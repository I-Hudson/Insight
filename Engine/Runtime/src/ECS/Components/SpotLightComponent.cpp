#include "ECS/Components/SpotLightComponent.h"

namespace Insight
{
	namespace ECS
	{
		SpotLightComponent::SpotLightComponent()
		{
		}

		SpotLightComponent::~SpotLightComponent()
		{ }

		IS_SERIALISABLE_CPP(SpotLightComponent)
	}
}