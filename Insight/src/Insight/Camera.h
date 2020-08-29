#pragma once

#include "Core.h"

#include <../vendor/glm/glm/glm.hpp>

class MeshComponent;

namespace Insight
{
	enum CameraAspect
	{
		A_4x3,
		A_16x9,

		CurrentWindowSize
	};

	///
	// BASE CAMERA
	///	
	class IS_API Camera
	{
	public:
		Camera();
		~Camera();

		//Set the view matrix for the camera
		void SetViewMatrix(const glm::mat4& a_value);
		//Set the projection matrix for the camera
		void SetProjMatrix(const float& a_fov, const CameraAspect& a_aspect, const float& a_near, const float& a_far);

		//Set the position for the camera
		void SetPosition(const glm::vec3& a_value);
		void SetFov(const float& fov);
		float& GetFov() { return m_fov; }
		void Update(float a_deltaTime);

		//Get the projection view matrix
		const glm::mat4& GetProjViewMatrix() const;
		const glm::mat4& GetProjMatrix() const;
		const glm::mat4& GetViewMatrix() const;

		bool MeshInFrustrum(MeshComponent& meshCom);

		const float GetCamerAspect(const CameraAspect& cameraAspect);

	private:

		void SetProjectionViewMatrix();

		float m_fov;
		CameraAspect m_cameraAspect;
		float m_nearPlane;
		float m_farPlane;

		glm::mat4 m_projectionViewMatrix;
		glm::mat4 m_projectionMatrix;
		glm::mat4 m_viewMatrix;

	};
}