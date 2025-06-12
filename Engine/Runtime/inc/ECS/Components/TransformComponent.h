#pragma once

#include "ECS/Entity.h"

#include "Maths/Matrix4.h"
#include "Serialisation/MathsSerialisation.h"

#include "ECS/Components/Generated/TransformComponent_reflect_generated.h"

namespace Insight
{
	namespace ECS
	{
		REFLECT_CLASS()
		class IS_RUNTIME TransformComponent : public Component
		{
			REFLECT_GENERATED_BODY()
		public:
			IS_COMPONENT(TransformComponent);

			TransformComponent();
			virtual ~TransformComponent() override;

			/// @brief Return the world transform.
			/// @return Maths::Matrix4.
			Maths::Matrix4 GetTransform()         const;
			/// @brief Get the local transform.
			/// @return Maths::Matrix4
			Maths::Matrix4 GetLocalTransform()    const;
			/// @brief Return the previous frame world transform.
			/// @return 
			Maths::Matrix4 GetPreviousTransform() const { return m_previous_transform; }

			/// @brief Return the Position.
			/// @return Maths::Vector3.
			Maths::Vector3 GetPosition() const;
			Maths::Quaternion GetRotation() const;
			Maths::Vector3 GetScale() const;

			/// @brief Set the Transform.
			/// @param transform 
			void SetTransform(const Maths::Matrix4& transform);
			/// @brief Set the Position.
			/// @param position 
			void SetPosition(const Maths::Vector3& position);
			void SetRotation(const Maths::Quaternion& rotation);
			void SetScale(const Maths::Vector3& scale);

			// Component
			virtual void OnUpdate(const float delta_time) override;

			IS_SERIALISABLE_H(TransformComponent)

		private:
			REFLECT_PROPERTY()
			Maths::Quaternion m_rotation = Maths::Quaternion::Identity;
			REFLECT_PROPERTY()
			Maths::Vector3 m_position = Maths::Vector3::Zero;
			REFLECT_PROPERTY()
			Maths::Vector3 m_scale = Maths::Vector3::One;

			
			//Maths::Matrix4 m_transform = Maths::Matrix4::Identity;
			Maths::Matrix4 m_previous_transform = Maths::Matrix4::Identity;
		};
	}
	OBJECT_SERIALISER(ECS::TransformComponent, 5,
		SERIALISE_PROPERTY_REMOVED(Maths::Matrix4, m_transform, 4, 5)
		SERIALISE_PROPERTY(Maths::Quaternion, m_rotation, 5, 0)
		SERIALISE_PROPERTY(Maths::Vector3, m_position, 5, 0)
		SERIALISE_PROPERTY(Maths::Vector3, m_scale, 5, 0)
		SERIALISE_BASE(ECS::Component, 3, 0)
	);
}