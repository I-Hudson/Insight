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
	}
}