#pragma once

#include "Runtime/Defines.h"
#include "ECS/Entity.h"
#include "Graphics/Frustum.h"

#include <glm/ext/matrix_float4x4.hpp>

namespace Insight
{
	namespace ECS
	{
		enum class IS_RUNTIME CameraType
		{
			Unknown,
			Perspective,
			Orthographic
		};

		class IS_RUNTIME Camera
		{
		public:
			Camera() = default;
			~Camera() = default;

			void CreatePerspective(float fovy, float aspect, float nearPlane, float farPlane);
			void CreateOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

			void SetInvertViewMatrix(bool invertViewMatrix);
			void SetViewMatrix(glm::mat4 viewMatrix);

			void SetNearPlane(float nearPlane);
			void SetFarPlane(float farPlane);
			void SetAspect(float aspect);
			void SetFovY(float fovy);

			glm::mat4 GetProjectionViewMatrix() const { return m_projectionView; }
			glm::mat4 GetProjectionMatrix()     const { return m_projection; }
			glm::mat4 GetViewMatrix()           const { return m_view; }

			CameraType GetCameraType()          const { return m_cameraType; }
			float GetNearPlane()                const { return m_nearPlane; }
			float GetFarPlane()                 const { return m_farPlane; }
			float GetAspect()                   const { return m_aspect; }
			float GetFovY()                     const { return m_fovY; }

		private:
			void ComputeProjectionMatrix();
			void ComputeProjectionViewMatrix();

		private:
			glm::mat4 m_projectionView = glm::mat4(1.0f);
			glm::mat4 m_projection = glm::mat4(1.0f);
			glm::mat4 m_view = glm::mat4(1.0f);

			CameraType m_cameraType = CameraType::Unknown;

			float m_nearPlane = 0.0f;
			float m_farPlane = 1024.0f;
			float m_aspect = 1.0f;
			// Yertical fov in radians.
			float m_fovY = 90.0f;

			// Should this camera be inverting its view matrix when computing the projectionView matrix.
			bool m_invertView = false;
		};

		class IS_RUNTIME CameraComponent : public Component
		{
		public:
			static constexpr char* Type_Name = "CameraComponent";
			virtual const char* GetTypeName() override { return Type_Name; }

			CameraComponent();
			virtual ~CameraComponent() override;

			void CreatePerspective(float fovy, float aspect, float nearPlane, float farPlane);
			void CreateOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

			void SetNearPlane(float nearPlane);
			void SetFarPlane(float farPlane);
			void SetAspect(float aspect);
			void SetFovY(float fovy);

			glm::mat4 GetProjectionMatrix() const { return m_camera.GetProjectionMatrix(); }
			glm::mat4 GetViewMatrix()       const;

			CameraType GetCameraType()      const { return m_camera.GetCameraType(); }
			float GetNearPlane()            const { return m_camera.GetNearPlane(); }
			float GetFarPlane()             const { return m_camera.GetFarPlane(); }
			float GetAspect()               const { return m_camera.GetAspect(); }
			float GetFovY()                 const { return m_camera.GetFovY(); }

			Graphics::Frustum GetFrustum()  const;
			glm::mat4 GetProjectionViewMatrix();

		private:
			Camera m_camera;
		};
	}
}