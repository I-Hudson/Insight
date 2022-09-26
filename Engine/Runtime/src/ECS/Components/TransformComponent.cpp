#include "ECS/Components/TransformComponent.h"

namespace Insight
{
	namespace ECS
	{
		TransformComponent::TransformComponent()
		{ 
			m_removeable = false;
		}

		TransformComponent::~TransformComponent()
		{ }
	}
}