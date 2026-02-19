#include "ECS/Components/TransformComponent.h"

#include "Core/Profiler.h"

#include <queue>

namespace Insight
{
	namespace ECS
	{
		TransformComponent::TransformComponent()
		{ 
			m_removeable = false;
			m_allow_multiple = false;
			m_isDirty = true;
			m_previous_transform = Maths::Matrix4::Identity;
		}

		TransformComponent::~TransformComponent()
		{ }

		Maths::Matrix4 TransformComponent::GetTransform() const
		{
			IS_PROFILE_FUNCTION();

			return m_worldTransform;
			/*
			Maths::Matrix4 transform = GetLocalTransform();
			Entity* parentEntity = GetOwnerEntity()->GetParent();
			while (parentEntity != nullptr)
			{
				Maths::Matrix4 parentTransform = parentEntity->GetComponent<ECS::TransformComponent>()->GetTransform();
				transform = transform * parentTransform;
				parentEntity = parentEntity->GetParent();
			}
			return transform;
			*/
		}

		Maths::Matrix4 TransformComponent::GetLocalTransform() const
		{
			const Maths::Matrix4 translateMatrix = Maths::Matrix4::Identity.Translated(Maths::Vector4(m_position, 0.0f));
			const Maths::Matrix4 rotationMatrix = Maths::Matrix4::Identity.Rotated(m_rotation);
			const Maths::Matrix4 scaleMatrix = Maths::Matrix4::Identity.Scaled(Maths::Vector4(m_scale, 0.0f));

			const Maths::Matrix4 transform = translateMatrix * rotationMatrix * scaleMatrix;
			return transform;
		}

		Maths::Vector3 TransformComponent::GetPosition() const
		{
			return m_position;
		}

		Maths::Quaternion TransformComponent::GetRotation() const
		{
			return m_rotation;
		}

		Maths::Vector3 TransformComponent::GetScale() const
		{
			return m_scale;
		}

		void TransformComponent::SetTransform(const Maths::Matrix4& transform)
		{
			Maths::Vector4 position;
			Maths::Quaternion rotation;
			Maths::Vector4 scale;
			transform.Decompose(position, rotation, scale);

			m_position = position;
			m_rotation = rotation;
			m_scale = scale;

			UpdateTransform();

			SetDirty();
		}

		void TransformComponent::SetPosition(const Maths::Vector3& position)
		{
			m_position = position;
			UpdateTransform();
			SetDirty();
		}

		void TransformComponent::SetRotation(const Maths::Quaternion& rotation)
		{
			m_rotation = rotation;
			UpdateTransform();
			SetDirty();
		}

		void TransformComponent::SetScale(const Maths::Vector3& scale)
		{
			m_scale = scale;
			UpdateTransform();
			SetDirty();
		}

		void TransformComponent::OnUpdate(const float delta_time)
		{
			if (m_isDirty)
			{
				UpdateTransform();
				m_isDirty = false;
			}
		}

		void TransformComponent::UpdateTransform(const bool updateChildren)
		{
			IS_PROFILE_FUNCTION();

			Maths::Matrix4 transform = GetLocalTransform();
			Entity* parentEntity = GetOwnerEntity()->GetParent();
			while (parentEntity != nullptr)
			{
				Maths::Matrix4 parentTransform = parentEntity->GetComponent<ECS::TransformComponent>()->GetTransform();
				transform = transform * parentTransform;
				parentEntity = parentEntity->GetParent();
			}
			m_previous_transform = m_worldTransform;
			m_worldTransform = transform;

			if (updateChildren)
			{
				IS_PROFILE_SCOPE("Update all children transforms");

				std::queue<Ptr<ECS::Entity>> transformToUpdate;
				transformToUpdate.push(GetOwnerEntity());

				while (!transformToUpdate.empty())
				{
					Ptr<ECS::Entity> parent = transformToUpdate.front();
					transformToUpdate.pop();

					const u32 childCount = parent->GetChildCount();
					for (size_t i = 0; i < childCount; i++)
					{
						Ptr<ECS::Entity> child = parent->GetChild(i);
						transformToUpdate.push(child);

						TransformComponent* transform = child->GetComponent<TransformComponent>();
						transform->UpdateTransform(false);
					}
				}
			}
		}

		IS_SERIALISABLE_CPP(TransformComponent)
	}
}