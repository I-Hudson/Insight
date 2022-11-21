#include "ECS/Components/FreeCameraControllerComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "Input/InputManager.h"

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
			Component* component = ownerEntity->GetComponentByName(CameraComponent::Type_Name);
			if (!component)
			{
				IS_WARN("[FreeCameraControllerComponent::OnBegin] No Camera component found on entity guid: '{0}', name: '{1}'.", ownerEntity->GetGUID().ToString(), ownerEntity->GetName());
				return;
			}

			CameraComponent* cameraComponent = static_cast<CameraComponent*>(component);
			if (!cameraComponent)
			{
				IS_WARN("[FreeCameraControllerComponent::OnBegin] Cast to camera component failed. Component type is '{0}'.", component->GetTypeName());
				return;
			}
			m_cameraComponent = cameraComponent;

			m_transformComponent = static_cast<TransformComponent*>(ownerEntity->GetComponentByName(TransformComponent::Type_Name));
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

			float frameSpeed = Input::InputManager::IsKeyPressed(IS_KEY_LEFT_SHIFT) ? deltaTime * 200 : deltaTime * 25;
			///Input::IsKeyDown(KEY_LEFT_SHIFT) ? a_deltaTime * m_cameraSpeed * 2 : a_deltaTime * m_cameraSpeed;

			/// Translate camera
			if (Input::InputManager::IsKeyPressed(IS_KEY_W))
			{
				vTranslation += vForward * frameSpeed;
			}
			if (Input::InputManager::IsKeyPressed(IS_KEY_S))
			{
				vTranslation -= vForward * frameSpeed;
			}
			if (Input::InputManager::IsKeyPressed(IS_KEY_D))
			{
				vTranslation += vRight * frameSpeed;
			}
			if (Input::InputManager::IsKeyPressed(IS_KEY_A))
			{
				vTranslation -= vRight * frameSpeed;
			}
			if (Input::InputManager::IsKeyPressed(IS_KEY_Q))
			{
				vTranslation += vUp * frameSpeed;
			}
			if (Input::InputManager::IsKeyPressed(IS_KEY_E))
			{
				vTranslation -= vUp * frameSpeed;
			}

			/// check for camera rotation
			static bool sbMouseButtonDown = false;
			bool mouseDown = Input::InputManager::IsMouseButtonPressed(IS_MOUSE_BUTTON_RIGHT);
			if (mouseDown)
			{
				viewMatrix[3] = vTranslation;

				static float siPrevMouseX = 0;
				static float siPrevMouseY = 0;

				if (sbMouseButtonDown == false)
				{
					sbMouseButtonDown = true;
					Input::InputManager::GetMousePosition(siPrevMouseX, siPrevMouseY);
				}

				float mouseX = 0, mouseY = 0;
				Input::InputManager::GetMousePosition(mouseX, mouseY);

				float iDeltaX = mouseX - siPrevMouseX;
				float iDeltaY = mouseY - siPrevMouseY;

				siPrevMouseX = mouseX;
				siPrevMouseY = mouseY;

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
				sbMouseButtonDown = false;
			}

			m_transformComponent->SetTransform(viewMatrix);

			float aspect = (float)Graphics::Window::Instance().GetWidth() / (float)Graphics::Window::Instance().GetHeight();
			m_cameraComponent->SetAspect(std::max(0.1f, aspect));
			m_cameraComponent->CreatePerspective(m_cameraComponent->GetFovY(), aspect, m_cameraComponent->GetNearPlane(), m_cameraComponent->GetFarPlane());
		}
	}
}