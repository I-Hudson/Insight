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
			/// @brief Return the Position.
			/// @return Maths::Vector3.
			Maths::Vector3 GetPosition()          const;
			/// @brief Return the previous frame world transform.
			/// @return 
			Maths::Matrix4 GetPreviousTransform() const { return m_previous_transform; }

			/// @brief Set the Transform.
			/// @param transform 
			void SetTransform(Maths::Matrix4 transform) { m_transform = transform; }
			/// @brief Set the Position.
			/// @param position 
			void SetPosition(Maths::Vector3 position) { m_transform[3] = Maths::Vector4(position, 1.0f); }

			// Component
			virtual void OnUpdate(const float delta_time) override;

			IS_SERIALISABLE_H(TransformComponent)

		private:
			REFLECT_PROPERTY()
			Maths::Matrix4 m_transform = Maths::Matrix4::Identity;
			Maths::Matrix4 m_previous_transform = Maths::Matrix4::Identity;
		};
	}
	OBJECT_SERIALISER(ECS::TransformComponent, 3,
		//SERIALISE_PROPERTY(Maths::Matrix4, m_transform, 2, 0)
		SERIALISE_BASE(ECS::Component, 3, 0)
	);
}