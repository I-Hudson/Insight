#include "ECS/Components/FreeCameraControllerComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

#include "Graphics/Window.h"

#include "Core/Logger.h"

#include <glm/gtx/matrix_interpolation.hpp>

namespace Insight
{
	namespace ECS
	{
		FreeCameraControllerComponent::FreeCameraControllerComponent()
		{ }

		FreeCameraControllerComponent::~FreeCameraControllerComponent()
		{ }

		void FreeCameraControllerComponent::OnBegin()
		{
			Entity* ownerEntity = GetOwnerEntity();
			m_transformComponent = static_cast<TransformComponent*>(ownerEntity->GetComponentByName(TransformComponent::Type_Name));
			
			Component* component = ownerEntity->GetComponentByName(CameraComponent::Type_Name);
			if (!component)
			{
				IS_LOG_WARN("[FreeCameraControllerComponent::OnBegin] No Camera component found on entity guid: '{0}', name: '{1}'.", ownerEntity->GetGUID().ToString(), ownerEntity->GetName());
				return;
			}

			CameraComponent* cameraComponent = static_cast<CameraComponent*>(component);
			if (!cameraComponent)
			{
				IS_LOG_WARN("[FreeCameraControllerComponent::OnBegin] Cast to camera component failed. Component type is '{0}'.", component->GetTypeName());
				return;
			}
			m_cameraComponent = cameraComponent;
		}

		void FreeCameraControllerComponent::OnUpdate(const float delta_time)
		{
			const float deltaTime = delta_time;
			bool negative_viewport = false;

			glm::mat4 viewMatrix = m_transformComponent->GetTransform();

			/// Get the camera's forward, right, up, and location vectors
			glm::vec4 vForward = viewMatrix[2];
			glm::vec4 vRight = viewMatrix[0];
			glm::vec4 vUp = viewMatrix[1];
			glm::vec4 vTranslation = viewMatrix[3];

			Input::InputDevice_KeyboardMouse* device = Input::InputSystem::Instance().GetKeyboardMouseDevice();
			float frameSpeed = device->WasHeld(Input::KeyboardButtons::Key_LShift) ? deltaTime * 200 : deltaTime * 25;
			///Input::IsKeyDown(KEY_LEFT_SHIFT) ? a_deltaTime * m_cameraSpeed * 2 : a_deltaTime * m_cameraSpeed;

			/// Translate camera
			if (device->WasHeld(Input::KeyboardButtons::Key_W))
			{
				vTranslation += vForward * frameSpeed;
			}
			if (device->WasHeld(Input::KeyboardButtons::Key_S))
			{
				vTranslation -= vForward * frameSpeed;
			}
			if (device->WasHeld(Input::KeyboardButtons::Key_D))
			{
				vTranslation += vRight * frameSpeed;
			}
			if (device->WasHeld(Input::KeyboardButtons::Key_A))
			{
				vTranslation -= vRight * frameSpeed;
			}
			if (device->WasHeld(Input::KeyboardButtons::Key_Q))
			{
				vTranslation += vUp * frameSpeed;
			}
			if (device->WasHeld(Input::KeyboardButtons::Key_E))
			{
				vTranslation -= vUp * frameSpeed;
			}

			/// check for camera rotation
			bool mouseDown = device->WasHeld(Input::MouseButtons::Right);
			if (mouseDown)
			{
				viewMatrix[3] = vTranslation;

				if (m_lookMoveEnabled == false)
				{
					m_lookMoveEnabled = true;
					m_previousLookX = device->GetMouseXPosition();
					m_previousLookY = device->GetMouseYPosition();
				}

				float mouseX = 0, mouseY = 0;
				mouseX = device->GetMouseXPosition();
				mouseY = device->GetMouseYPosition();

				float iDeltaX = mouseX - m_previousLookX;
				float iDeltaY = mouseY - m_previousLookY;

				m_previousLookX = mouseX;
				m_previousLookY = mouseY;

				glm::mat4 mMat;

				/// pitch
				if (iDeltaY != 0)
				{
					float i_delta_y = static_cast<float>(iDeltaY);
					if (negative_viewport)
					{
						i_delta_y = -i_delta_y;
					}
					mMat = glm::axisAngleMatrix(vRight.xyz(), i_delta_y / 150.0f);
					vRight = mMat * vRight;
					vUp = mMat * vUp;
					vForward = mMat * vForward;
				}

				/// yaw
				if (iDeltaX != 0)
				{
					float i_delta_x = static_cast<float>(iDeltaX);
					mMat = glm::axisAngleMatrix(glm::vec3(0, 1, 0), i_delta_x / 150.0f);
					vRight = mMat * vRight;
					vUp = mMat * vUp;
					vForward = mMat * vForward;
				}

				viewMatrix[0] = vRight;
				viewMatrix[1] = vUp;
				viewMatrix[2] = vForward;

			}
			else
			{
				m_lookMoveEnabled = false;
			}

			m_transformComponent->SetTransform(viewMatrix);

			float aspect = (float)Graphics::Window::Instance().GetWidth() / (float)Graphics::Window::Instance().GetHeight();

			if (m_cameraComponent)
			{
				m_cameraComponent->SetAspect(std::max(0.1f, aspect));
				m_cameraComponent->CreatePerspective(m_cameraComponent->GetFovY(), aspect, m_cameraComponent->GetNearPlane(), m_cameraComponent->GetFarPlane());
			}
		}
	}
}