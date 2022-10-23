#pragma once

#include "ECS/Entity.h"

#include <glm/glm.hpp>

namespace Insight
{
	namespace ECS
	{
		class TransformComponent : public Component
		{
		public:
			static constexpr char* Type_Name = "TransformComponent";
			virtual const char* GetTypeName() override { return Type_Name; }

			TransformComponent();
			~TransformComponent();

			/// @brief Return the transform.
			/// @return glm::mat4.
			glm::mat4 GetTransform() const { return m_transform; }
			/// @brief Return the Position.
			/// @return glm::vec3.
			glm::vec3 GetPosition() const { return m_transform[3].xyz; }

			/// @brief Set the Transform.
			/// @param transform 
			void SetTransform(glm::mat4 transform) { m_transform = transform; }
			/// @brief Set the Position.
			/// @param position 
			void SetPosition(glm::vec3 position) { m_transform[3] = glm::vec4(position, 1.0f); }

			// Component
			virtual void OnUpdate(const float delta_time) override;

		private:
			glm::mat4 m_transform = glm::mat4(1.0f);
			glm::mat4 m_previous_transform = glm::mat4(1.0f);
		};
	}
}