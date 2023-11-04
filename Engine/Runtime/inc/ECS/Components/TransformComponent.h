#pragma once

#include "ECS/Entity.h"

#include "ECS/Components/Generated/TransformComponent_reflect_generated.h"

#include <glm/glm.hpp>

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
			/// @return glm::mat4.
			glm::mat4 GetTransform()         const;
			/// @brief Get the local transform.
			/// @return glm::mat4
			glm::mat4 GetLocalTransform()    const;
			/// @brief Return the Position.
			/// @return glm::vec3.
			glm::vec3 GetPosition()          const;
			/// @brief Return the previous frame world transform.
			/// @return 
			glm::mat4 GetPreviousTransform() const { return m_previous_transform; }

			/// @brief Set the Transform.
			/// @param transform 
			void SetTransform(glm::mat4 transform) { m_transform = transform; }
			/// @brief Set the Position.
			/// @param position 
			void SetPosition(glm::vec3 position) { m_transform[3] = glm::vec4(position, 1.0f); }

			// Component
			virtual void OnUpdate(const float delta_time) override;

			IS_SERIALISABLE_H(TransformComponent)

		private:
			REFLECT_PROPERTY()
			glm::mat4 m_transform = glm::mat4(1.0f);
			glm::mat4 m_previous_transform = glm::mat4(1.0f);
		};
	}
	OBJECT_SERIALISER(ECS::TransformComponent, 3,
		SERIALISE_PROPERTY(glm::mat4, m_transform, 2, 0)
		SERIALISE_BASE(ECS::Component, 3, 0)
	);
}