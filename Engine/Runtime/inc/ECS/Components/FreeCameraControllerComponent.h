#pragma once

#include "Runtime/Defines.h"
#include "ECS/Entity.h"

#include <glm/ext/matrix_float4x4.hpp>

namespace Insight
{
	namespace ECS
	{
		class CameraComponent;
		class TransformComponent;

		class IS_RUNTIME FreeCameraControllerComponent : public Component
		{
		public:
			IS_COMPONENT(FreeCameraControllerComponent);

			FreeCameraControllerComponent();
			virtual ~FreeCameraControllerComponent() override;

			virtual void OnBegin() override;
			virtual void OnUpdate(const float delta_time) override;

		private:
			CameraComponent* m_cameraComponent = nullptr;
			TransformComponent* m_transformComponent = nullptr;

			bool m_lookMoveEnabled = false;
			float m_previousLookX = 0.0f;
			float m_previousLookY = 0.0f;
		};
	}
}