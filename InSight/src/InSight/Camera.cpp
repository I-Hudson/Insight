#include "ispch.h"

#include "Camera.h"

#include "Input/Input.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

namespace Framework
{
	Camera::Camera()
		: m_viewMatrix(glm::mat4(1.0f)), m_projectionMatrix(glm::mat4(1.0f))
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::SetViewMatrix(const glm::mat4& a_value)
	{
		m_viewMatrix = a_value;
		SetProjectionViewMatrix();
	}

	void Camera::SetProjMatrix(const glm::mat4& a_value)
	{
		m_projectionMatrix = a_value;
		SetProjectionViewMatrix();
	}

	void Camera::SetProjMatrix(const float& a_fov, const float& a_aspect, const float& a_near, const float& a_far)
	{
		m_projectionMatrix = glm::perspective(a_fov, a_aspect, a_near, a_far);
		SetProjectionViewMatrix();
	}

	void Camera::SetPosition(const glm::vec3& a_value)
	{
		SetProjectionViewMatrix();
	}

	void Camera::Update(float a_deltaTime)
	{
		// Get the camera's forward, right, up, and location vectors
		glm::vec4 vForward = m_viewMatrix[2];
		glm::vec4 vRight = m_viewMatrix[0];
		glm::vec4 vUp = m_viewMatrix[1];
		glm::vec4 vTranslation = m_viewMatrix[3];

		float frameSpeed = Input::KeyDown(KEY_LEFT_SHIFT) ? a_deltaTime * 5 * 2 : a_deltaTime * 5;

		// Translate camera
		if (Input::KeyDown('w'))
		{
			vTranslation -= vForward * frameSpeed;
		}
		if (Input::KeyDown('S'))
		{
			vTranslation += vForward * frameSpeed;
		}
		if (Input::KeyDown('d'))
		{
			vTranslation += vRight * frameSpeed;
		}
		if (Input::KeyDown('a'))
		{
			vTranslation -= vRight * frameSpeed;
		}
		if (Input::KeyDown('q'))
		{
			vTranslation += vUp * frameSpeed;
		}
		if (Input::KeyDown('e'))
		{
			vTranslation -= vUp * frameSpeed;
		}

		m_viewMatrix[3] = vTranslation;

		// check for camera rotation
		static bool sbMouseButtonDown = false;
		if (Input::KeyDown(MOUSE_BUTTON_RIGHT))
		{
			static double siPrevMouseX = 0;
			static double siPrevMouseY = 0;

			if (sbMouseButtonDown == false)
			{
				sbMouseButtonDown = true;
				Input::GetMousePosition(&siPrevMouseX, &siPrevMouseY);
			}

			double mouseX = 0, mouseY = 0;
			Input::GetMousePosition(&mouseX, &mouseY);

			double iDeltaX = mouseX - siPrevMouseX;
			double iDeltaY = mouseY - siPrevMouseY;

			siPrevMouseX = mouseX;
			siPrevMouseY = mouseY;

			glm::mat4 mMat;

			// pitch
			if (iDeltaY != 0)
			{
				mMat = glm::axisAngleMatrix(vRight.xyz(), (float)-iDeltaY / 150.0f);
				vRight = mMat * vRight;
				vUp = mMat * vUp;
				vForward = mMat * vForward;
			}

			// yaw
			if (iDeltaX != 0)
			{
				mMat = glm::axisAngleMatrix(glm::vec3(0,1,0), (float)-iDeltaX / 150.0f);
				vRight = mMat * vRight;
				vUp = mMat * vUp;
				vForward = mMat * vForward;
			}

			m_viewMatrix[0] = vRight;
			m_viewMatrix[1] = vUp;
			m_viewMatrix[2] = vForward;
		}
		else
		{
			sbMouseButtonDown = false;
		}
		SetProjectionViewMatrix();
	}

	const glm::mat4& Camera::GetProjViewMatrix() const
	{
		return m_projectionViewMatrix; 
	}

	const glm::mat4& Camera::GetProjMatrix() const
	{
		return m_projectionMatrix;
	}

	const glm::mat4& Camera::GetViewMatrix() const
	{
		return m_viewMatrix;
	}

	void Camera::SetProjectionViewMatrix()
	{
		m_projectionViewMatrix = m_projectionMatrix * glm::inverse(m_viewMatrix);
	}
}