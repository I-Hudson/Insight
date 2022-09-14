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

			glm::mat4 GetTransform() const { return m_transform; }
			glm::vec3 GetPosition() const { return m_transform[3].xyz; }

			void SetTransform(glm::mat4 transform) { m_transform = transform; }
			void SetPosition(glm::vec3 position) { m_transform[3] = glm::vec4(position, 1.0f); }

		private:
			glm::mat4 m_transform;
		};
	}
}