#include "ECS/Components/TransformComponent.h"

namespace Insight
{
	namespace ECS
	{
		TransformComponent::TransformComponent()
		{ 
			m_removeable = false;
			m_allow_multiple = false;
			m_previous_transform = glm::mat4(1.0f);
		}

		TransformComponent::~TransformComponent()
		{ }

		void TransformComponent::OnUpdate(const float delta_time)
		{
			m_previous_transform = m_transform;
		}
	}
}