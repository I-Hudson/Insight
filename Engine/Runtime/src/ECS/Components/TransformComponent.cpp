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

		void TransformComponent::OnUpdate(const float delta_time)
		{
			m_previous_transform = m_transform;
		}
	}
}