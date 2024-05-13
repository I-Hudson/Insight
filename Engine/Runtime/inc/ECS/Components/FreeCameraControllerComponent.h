#pragma once

#include "Runtime/Defines.h"
#include "ECS/Entity.h"

#include "ECS/Components/CameraComponent.h"

#include "Generated/FreeCameraControllerComponent_reflect_generated.h"

namespace Insight
{
	namespace ECS
	{
		class TransformComponent;

		REFLECT_CLASS()
		class IS_RUNTIME FreeCameraControllerComponent : public Component
		{
			REFLECT_GENERATED_BODY()
		public:
			IS_COMPONENT(FreeCameraControllerComponent);

			FreeCameraControllerComponent();
			virtual ~FreeCameraControllerComponent() override;

			virtual void OnBegin() override;
			virtual void OnUpdate(const float delta_time) override;

		private:
			REFLECT_PROPERTY(EditorVisible)
			CameraComponent* m_cameraComponent = nullptr;
			TransformComponent* m_transformComponent = nullptr;

			bool m_lookMoveEnabled = false;
			float m_previousLookX = 0.0f;
			float m_previousLookY = 0.0f;
		};
	}
}