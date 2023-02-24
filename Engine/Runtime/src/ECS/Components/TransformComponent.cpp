#include "ECS/Components/TransformComponent.h"

#include "Core/Profiler.h"

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

		glm::mat4 TransformComponent::GetTransform() const
		{
			IS_PROFILE_FUNCTION();
			glm::mat4 transform = m_transform;
			Entity* parentEntity = GetOwnerEntity()->GetParent();
			while (parentEntity != nullptr)
			{
				glm::mat4 parentTransform = parentEntity->GetComponent<ECS::TransformComponent>()->GetTransform();
				transform = transform * parentTransform;
				parentEntity = parentEntity->GetParent();
			}
			return transform;
		}

		glm::mat4 TransformComponent::GetLocalTransform() const
		{
			return m_transform;
		}

		glm::vec3 TransformComponent::GetPosition() const
		{
			return GetTransform()[3];
		}

		void TransformComponent::OnUpdate(const float delta_time)
		{
			m_previous_transform = m_transform;
		}

		IS_SERIALISABLE_CPP(TransformComponent)
	}
}