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
			m_previous_transform = Maths::Matrix4::Identity;
		}

		TransformComponent::~TransformComponent()
		{ }

		Maths::Matrix4 TransformComponent::GetTransform() const
		{
			IS_PROFILE_FUNCTION();
			Maths::Matrix4 transform = m_transform;
			Entity* parentEntity = GetOwnerEntity()->GetParent();
			while (parentEntity != nullptr)
			{
				Maths::Matrix4 parentTransform = parentEntity->GetComponent<ECS::TransformComponent>()->GetTransform();
				transform = transform * parentTransform;
				parentEntity = parentEntity->GetParent();
			}
			return transform;
		}

		Maths::Matrix4 TransformComponent::GetLocalTransform() const
		{
			return m_transform;
		}

		Maths::Vector3 TransformComponent::GetPosition() const
		{
			return GetTransform()[3];
		}

		Maths::Quaternion TransformComponent::GetRotation() const
		{
			Maths::Vector4 position;
			Maths::Quaternion rotation;
			Maths::Vector4 scale;
			m_transform.Decompose(position, rotation, scale);
			return rotation;
		}

		void TransformComponent::OnUpdate(const float delta_time)
		{
			m_previous_transform = m_transform;
		}

		IS_SERIALISABLE_CPP(TransformComponent)
	}
}