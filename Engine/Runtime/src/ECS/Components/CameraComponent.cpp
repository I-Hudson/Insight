#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Insight
{
	namespace ECS
	{
		//----------------------------------------------------
		// Camera
		//----------------------------------------------------
		void Camera::CreatePerspective(float fovy, float aspect, float nearPlane, float farPlane)
		{
			m_cameraType = CameraType::Perspective;
			m_fovY = fovy;
			m_aspect = aspect;
			m_nearPlane = nearPlane;
			m_farPlane = farPlane;
			ComputeProjectionMatrix();
		}

		void Camera::CreateOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
		{
			m_cameraType = CameraType::Orthographic;
			m_nearPlane = nearPlane;
			m_farPlane = farPlane;
			m_projection = glm::ortho(left, right, bottom, top, m_nearPlane, m_farPlane);
			ComputeProjectionMatrix();
		}

		void Camera::SetInvertViewMatrix(bool invertViewMatrix)
		{
			m_invertView = invertViewMatrix;
		}

		void Camera::SetViewMatrix(glm::mat4 viewMatrix)
		{
			m_view = viewMatrix;
			ComputeProjectionViewMatrix();
			ComputeFrustm();
		}

		void Camera::SetNearPlane(float nearPlane)
		{
			m_nearPlane = nearPlane;
			ComputeProjectionMatrix();
		}

		void Camera::SetFarPlane(float farPlane)
		{
			m_farPlane = farPlane;
			ComputeProjectionMatrix();
		}

		void Camera::SetAspect(float aspect)
		{
			m_aspect = aspect;
			ComputeProjectionMatrix();
		}

		void Camera::SetFovY(float fovy)
		{
			m_fovY = fovy;
			ComputeProjectionMatrix();
		}

		glm::mat4 Camera::GetInvertedProjectionViewMatrix() const
		{
			if (m_invertView)
			{
				return glm::inverse(m_projection * glm::inverse(m_view));
			}
			else
			{
				return glm::inverse(m_projection * m_view);
			}
		}

		bool Camera::IsVisible(const glm::vec3& center, const glm::vec3& extent, bool ignore_near_plane /*= false*/) const
		{
			return m_frusum.IsVisible(center, extent, ignore_near_plane);
		}

		bool Camera::IsVisible(const Graphics::BoundingBox& boundingbox) const
		{
			return IsVisible(boundingbox.GetCenter(), boundingbox.GetExtents());
		}

		void Camera::ComputeProjectionMatrix()
		{
			if (m_cameraType == CameraType::Perspective)
			{
				m_aspect = std::max(0.1f, m_aspect);
				m_projection = glm::perspective(m_fovY, m_aspect, m_nearPlane, m_farPlane);
			}
			ComputeProjectionViewMatrix();
			ComputeFrustm();
		}

		void Camera::ComputeProjectionViewMatrix()
		{
			glm::mat4 proj = m_projection;
			if (Graphics::RenderContext::Instance().GetGraphicsAPI() == Graphics::GraphicsAPI::Vulkan)
			{
				proj[1][1] *= -1;
			}

			if (m_invertView)
			{
				m_projectionView = proj * glm::inverse(m_view);
			}
			else
			{
				m_projectionView = proj * m_view;
			}
		}

		void Camera::ComputeFrustm()
		{
			m_frusum = Graphics::Frustum(m_view, m_projection, m_farPlane);
		}



		//----------------------------------------------------
		// CameraComponent
		//----------------------------------------------------
		CameraComponent::CameraComponent()
		{
			m_allow_multiple = false;
			m_camera.SetInvertViewMatrix(true);
		}

		CameraComponent::~CameraComponent()
		{
		}

		IS_SERIALISABLE_CPP(CameraComponent)

		void CameraComponent::OnCreate()
		{
			const float aspect = (float)Graphics::Window::Instance().GetWidth() / (float)Graphics::Window::Instance().GetHeight();
			CreatePerspective(glm::radians(90.0f), aspect, 0.1f, 1024.0f);
		}

		void CameraComponent::CreatePerspective(float fovy, float aspect, float nearPlane, float farPlane)
		{
			m_camera.CreatePerspective(fovy, aspect, nearPlane, farPlane);
		}

		void CameraComponent::CreateOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
		{
			m_camera.CreateOrthographic(left, right, bottom, top, nearPlane, farPlane);
		}

		void CameraComponent::SetNearPlane(float nearPlane)
		{
			m_camera.SetNearPlane(nearPlane);
		}

		void CameraComponent::SetFarPlane(float farPlane)
		{
			m_camera.SetFarPlane(farPlane);
		}

		void CameraComponent::SetAspect(float aspect)
		{
			m_camera.SetAspect(aspect);
		}

		void CameraComponent::SetFovY(float fovy)
		{
			m_camera.SetFovY(fovy);
		}

		glm::mat4 CameraComponent::GetProjectionViewMatrix() const
		{
			// Call 'GetViewMatrix' to update the camera's view matrix.
			GetViewMatrix();
			return m_camera.GetProjectionViewMatrix();
		}

		glm::mat4 CameraComponent::GetViewMatrix() const
		{
			TransformComponent* transformComponent = static_cast<TransformComponent*>(GetOwnerEntity()->GetComponentByName(TransformComponent::Type_Name));
			if (transformComponent != nullptr)
			{
				const_cast<Camera&>(m_camera).SetViewMatrix(transformComponent->GetTransform());
			}
			else
			{
				const_cast<Camera&>(m_camera).SetViewMatrix(glm::mat4(1.0f));
			}
			return m_camera.GetViewMatrix();
		}

		Graphics::Frustum CameraComponent::GetFrustum() const
		{
			return Graphics::Frustum(GetViewMatrix(), m_camera.GetProjectionMatrix(), GetFarPlane());
		}
	}
}