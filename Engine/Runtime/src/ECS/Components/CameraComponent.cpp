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
		IS_SERIALISABLE_CPP(Camera)

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
			m_projection = Maths::Matrix4::CreateOrthographic(left, right, bottom, top, 0, std::numeric_limits<float>::max());
			ComputeProjectionMatrix();
		}

		void Camera::SetInvertViewMatrix(bool invertViewMatrix)
		{
			m_invertView = invertViewMatrix;
		}

		void Camera::SetViewMatrix(Maths::Matrix4 viewMatrix)
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

		Maths::Matrix4 Camera::GetInvertedProjectionViewMatrix() const
		{
			if (m_invertView)
			{
				float _00 = m_view[0][0];
				float _01 = m_view[0][1];
				float _02 = m_view[0][2];
				float _03 = m_view[0][3];

				float _10 = m_view[1][0];
				float _11 = m_view[1][1];
				float _12 = m_view[1][2];
				float _13 = m_view[1][3];

				float _20 = m_view[2][0];
				float _21 = m_view[2][1];
				float _22 = m_view[2][2];
				float _23 = m_view[2][3];

				float _30 = m_view[3][0];
				float _31 = m_view[3][1];
				float _32 = m_view[3][2];
				float _33 = m_view[3][3];

				glm::mat4 glmView(
					m_view[0][0], m_view[0][1], m_view[0][2], m_view[0][3],
					m_view[1][0], m_view[1][1], m_view[1][2], m_view[1][3],
					m_view[2][0], m_view[2][1], m_view[2][2], m_view[2][3],
					m_view[3][0], m_view[3][1], m_view[3][2], m_view[3][3]);

				glm::mat4 glmProj(
					m_projection[0][0], m_projection[0][1], m_projection[0][2], m_projection[0][3],
					m_projection[1][0], m_projection[1][1], m_projection[1][2], m_projection[1][3],
					m_projection[2][0], m_projection[2][1], m_projection[2][2], m_projection[2][3],
					m_projection[3][0], m_projection[3][1], m_projection[3][2], m_projection[3][3]);

				glm::mat4 glmViewInv = glm::inverse(glmView);
				glm::mat4 glmProjView = glmProj * glmViewInv;
				glm::mat4 glmProjViewInv =  glm::inverse(glmProjView);

				Maths::Matrix4 viewInv = m_view.Inversed();
				Maths::Matrix4 projView = m_projection * viewInv;
				Maths::Matrix4 projViewInv = projView.Inversed();
				return projViewInv;
			}
			else
			{
				return (m_projection * m_view).Inversed();
				//return glm::inverse(m_projection * m_view);
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
				m_projection = Maths::Matrix4::CreatePerspective(m_fovY, m_aspect, m_nearPlane, m_farPlane);
			}
			ComputeProjectionViewMatrix();
			ComputeFrustm();
		}

		void Camera::ComputeProjectionViewMatrix()
		{
			Maths::Matrix4 proj = m_projection;
			if (Graphics::RenderContext::Instance().GetGraphicsAPI() == Graphics::GraphicsAPI::Vulkan)
			{
				proj[1][1] *= -1;
			}

			if (m_invertView)
			{
				m_projectionView = proj * m_view.Inversed();
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

		Maths::Matrix4 CameraComponent::GetProjectionViewMatrix() const
		{
			// Call 'GetViewMatrix' to update the camera's view matrix.
			GetViewMatrix();
			return m_camera.GetProjectionViewMatrix();
		}

		Maths::Matrix4 CameraComponent::GetViewMatrix() const
		{
			TransformComponent* transformComponent = static_cast<TransformComponent*>(GetOwnerEntity()->GetComponentByName(TransformComponent::Type_Name));
			if (transformComponent != nullptr)
			{
				glm::mat4 transformMatrix = transformComponent->GetTransform();
				Maths::Matrix4 m(
					transformMatrix[0][0], transformMatrix[0][1], transformMatrix[0][2], transformMatrix[0][3],
					transformMatrix[1][0], transformMatrix[1][1], transformMatrix[1][2], transformMatrix[1][3],
					transformMatrix[2][0], transformMatrix[2][1], transformMatrix[2][2], transformMatrix[2][3],
					transformMatrix[3][0], transformMatrix[3][1], transformMatrix[3][2], transformMatrix[3][3]);
				const_cast<Camera&>(m_camera).SetViewMatrix(m);
			}
			else
			{
				const_cast<Camera&>(m_camera).SetViewMatrix(Maths::Matrix4::Identity);
			}
			return m_camera.GetViewMatrix();
		}

		Graphics::Frustum CameraComponent::GetFrustum() const
		{
			return Graphics::Frustum(GetViewMatrix(), m_camera.GetProjectionMatrix(), GetFarPlane());
		}
	}
}