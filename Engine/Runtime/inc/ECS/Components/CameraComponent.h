#pragma once

#include "Runtime/Defines.h"
#include "ECS/Entity.h"
#include "Graphics/Frustum.h"

#include "Maths/Matrix4.h"

#include "Generated/CameraComponent_reflect_generated.h"

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

		class IS_RUNTIME Camera : public Serialisation::ISerialisable
		{
		public:
			Camera() = default;
			~Camera() = default;

			IS_SERIALISABLE_H(Camera);

			void CreatePerspective(float fovy, float aspect, float nearPlane, float farPlane);
			void CreateOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

			void SetInvertViewMatrix(bool invertViewMatrix);
			void SetViewMatrix(const Maths::Matrix4 viewMatrix);

			void SetNearPlane(float nearPlane);
			void SetFarPlane(float farPlane);
			void SetAspect(float aspect);
			void SetFovY(float fovy);

			Maths::Matrix4 GetProjectionViewMatrix()			const { return m_projectionView; }
			Maths::Matrix4 GetProjectionMatrix()				const { return m_projection; }
			Maths::Matrix4 GetViewMatrix()						const { return m_view; }

			Maths::Matrix4 GetInvertedProjectionViewMatrix()	const;
			Maths::Matrix4 GetInvertedViewMatrix()				const { return m_view.Inversed(); }

			CameraType GetCameraType()							const { return m_cameraType; }
			float GetNearPlane()								const { return m_nearPlane; }
			float GetFarPlane()									const { return m_farPlane; }
			float GetAspect()									const { return m_aspect; }
			float GetFovY()										const { return m_fovY; }

			bool IsVisible(const glm::vec3& center, const glm::vec3& extent, bool ignore_near_plane = false) const;
			bool IsVisible(const Graphics::BoundingBox& boundingbox) const;

		private:
			void ComputeProjectionMatrix();
			void ComputeProjectionViewMatrix();
			/// @brief Compute the camera frustum.
			void ComputeFrustm();

		private:
			Maths::Matrix4 m_projectionView = Maths::Matrix4::Identity;
			Maths::Matrix4 m_projection = Maths::Matrix4::Identity;
			Maths::Matrix4 m_view = Maths::Matrix4::Identity;

			CameraType m_cameraType = CameraType::Unknown;
			Graphics::Frustum m_frusum;

			float m_nearPlane = 0.0f;
			float m_farPlane = 1024.0f;
			float m_aspect = 1.0f;
			// Yertical fov in radians.
			float m_fovY = 90.0f;

			// Should this camera be inverting its view matrix when computing the projectionView matrix.
			bool m_invertView = false;
		};

		REFLECT_CLASS()
		class IS_RUNTIME CameraComponent : public Component
		{
			REFLECT_GENERATED_BODY()
		public:
			IS_COMPONENT(CameraComponent);

			CameraComponent();
			virtual ~CameraComponent() override;

			IS_SERIALISABLE_H(CameraComponent)

			virtual void OnCreate() override;

			void CreatePerspective(float fovy, float aspect, float nearPlane, float farPlane);
			void CreateOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

			void SetNearPlane(float nearPlane);
			void SetFarPlane(float farPlane);
			void SetAspect(float aspect);
			void SetFovY(float fovy);

			Maths::Matrix4 GetProjectionViewMatrix()			const;
			Maths::Matrix4 GetProjectionMatrix()				const { return m_camera.GetProjectionMatrix(); }
			Maths::Matrix4 GetViewMatrix()						const;

			Maths::Matrix4 GetInvertedProjectionViewMatrix()	const { return m_camera.GetInvertedProjectionViewMatrix(); }
			Maths::Matrix4 GetInvertedViewMatrix()				const { return m_camera.GetInvertedViewMatrix(); }

			Camera GetCamera()									const { return m_camera; }
			CameraType GetCameraType()							const { return m_camera.GetCameraType(); }
			float GetNearPlane()								const { return m_camera.GetNearPlane(); }
			float GetFarPlane()									const { return m_camera.GetFarPlane(); }
			float GetAspect()									const { return m_camera.GetAspect(); }
			float GetFovY()										const { return m_camera.GetFovY(); }

			Graphics::Frustum GetFrustum()  const;

		private:
			Camera m_camera;
		};
	}
	OBJECT_SERIALISER(ECS::Camera, 1,
		SERIALISE_PROPERTY(ECS::CameraType, m_cameraType, 1, 0)
		SERIALISE_PROPERTY(float, m_nearPlane, 1, 0)
		SERIALISE_PROPERTY(float, m_farPlane, 1, 0)
		SERIALISE_PROPERTY(float, m_aspect, 1, 0)
		SERIALISE_PROPERTY(float, m_fovY, 1, 0)
		SERIALISE_PROPERTY(bool, m_invertView, 1, 0)
	);

	OBJECT_SERIALISER(ECS::CameraComponent, 3,
		SERIALISE_BASE(ECS::Component, 2, 0)
		SERIALISE_OBJECT(ECS::Camera, m_camera, 1, 0)
	);
}