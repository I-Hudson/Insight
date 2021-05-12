#include "ispch.h"
#include "Engine/Component/CameraComponent.h"

#include "Engine/Component/TransformComponent.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Input/Input.h"
#include "Engine/Module/WindowModule.h"
#include "Engine/Module/GraphicsModule.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

#include "Engine/RTTI/RTTI.h"

REGISTER_DEF_TYPE(CameraComponent);


CameraComponent::CameraComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity)
	: Component(componentManager, componentID, componentType, entityManager, entity)
	, m_cameraAspect(CameraAspect::CurrentWindowSize)
	, m_fov(0.0f)
	, m_nearPlane(0.0f)
	, m_farPlane(0.0f)
	, m_cameraSpeed(0.0f)
	, m_projectionViewMatrix(glm::mat4(1.0f))
	, m_projectionMatrix(glm::mat4(1.0f))
{
	SetType<CameraComponent>();
	//m_updateEveryFarme = true;
}

CameraComponent::~CameraComponent()
{
}

//void CameraComponent::OnCreate()
//{
//	__super::OnCreate();
//
//	IS_PROPERTY_GLOBAL(CameraComponent, "Camera Component", ShowInEditor);
//	IS_PROPERTY(float, m_fov, "FOV", ShowInEditor | ReadOnly);
//	IS_PROPERTY(float, m_nearPlane, "Near Plane", ShowInEditor | ClampZero);
//	IS_PROPERTY(float, m_farPlane, "Far Plane", ShowInEditor | ReadOnly);
//
//	m_cameraSpeed = 50;
//	SetProjMatrix(90, CameraAspect::CurrentWindowSize, 0.1f, 100000.0f);
//	SetViewMatrix(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
//
//	if (!Module::GraphicsModule::Instance()->HasMainCamera())
//	{
//		Module::GraphicsModule::Instance()->SetMainCamera(this);
//	}
//}

//void CameraComponent::OnDestroy()
//{
//	if (Module::GraphicsModule::IsInitialised() && Module::GraphicsModule::Instance()->IsThisMainCamera(this))
//	{
//		Module::GraphicsModule::Instance()->SetMainCamera(nullptr);
//	}
//}

//void CameraComponent::Serialize(Serialization::SerializableElement* element, bool force)
//{
//	element->AddAttribute("UUID", GetUUID());
//	element->AddAttribute("Type", "CameraComponent");
//	element->AddAttribute("FOV", std::to_string(m_fov));
//	element->AddAttribute("NearPlane", std::to_string(m_nearPlane));
//	element->AddAttribute("FarPlane", std::to_string(m_farPlane));
//}

//void CameraComponent::Deserialize(Serialization::SerializableElement* element, bool force)
//{
//	if (auto ptr = element->GetFirstAttribute("FOV"))
//	{
//		m_fov = std::stof(ptr->GetValue());
//	}
//	if (auto ptr = element->GetFirstAttribute("NearPlane"))
//	{
//		m_nearPlane = std::stof(ptr->GetValue());
//	}
//	if (auto ptr = element->GetFirstAttribute("FarPlane"))
//	{
//		m_farPlane = std::stof(ptr->GetValue());
//	}
//}

void CameraComponent::SetViewMatrix(const glm::mat4& a_value)
{
	GetEntity().GetComponent<TransformComponent>().SetTransform(a_value);
	SetProjectionViewMatrix();

}

void CameraComponent::SetProjMatrix(const float& a_fov, const CameraAspect& a_aspect, const float& a_near, const float& a_far)
{
	m_fov = a_fov;
	m_cameraAspect = a_aspect;
	m_nearPlane = a_near;
	m_farPlane = a_far;

	m_projectionMatrix = glm::perspective(glm::radians(m_fov), GetCamerAspect(a_aspect), a_near, a_far);
	SetProjectionViewMatrix();
}

void CameraComponent::SetProjMatrix(const float& a_fov, const float& a_near, const float& a_far)
{
	m_fov = a_fov;
	m_nearPlane = a_near;
	m_farPlane = a_far;

	m_projectionMatrix = glm::perspective(glm::radians(m_fov), GetCamerAspect(m_cameraAspect), a_near, a_far);
	SetProjectionViewMatrix();
}

void CameraComponent::SetPosition(const glm::vec3& a_value)
{
	SetProjectionViewMatrix();
}

void CameraComponent::SetFov(const float& fov)
{
	m_fov = fov;
	SetProjMatrix(m_fov, m_cameraAspect, m_nearPlane, m_farPlane);
}

void CameraComponent::SetCameraSpeed(const float& cameraSpeed)
{
	m_cameraSpeed = cameraSpeed;
}

void CameraComponent::OnUpdate(const float& a_deltaTime)
{
	SetProjMatrix(m_fov, m_cameraAspect, m_nearPlane, m_farPlane);


	glm::mat4 viewMatrix = GetEntity().GetComponent<TransformComponent>().GetTransform();

	// Get the camera's forward, right, up, and location vectors
	glm::vec4 vForward = viewMatrix[2];
	glm::vec4 vRight = viewMatrix[0];
	glm::vec4 vUp = viewMatrix[1];
	glm::vec4 vTranslation = viewMatrix[3];

	float frameSpeed = Input::IsKeyDown(KEY_LEFT_SHIFT) ? a_deltaTime * m_cameraSpeed * 2 : a_deltaTime * m_cameraSpeed;

	// Translate camera
	if (Input::IsKeyDown(KEY_W))
	{
		vTranslation -= vForward * frameSpeed;
	}
	if (Input::IsKeyDown(KEY_S))
	{
		vTranslation += vForward * frameSpeed;
	}
	if (Input::IsKeyDown(KEY_D))
	{
		vTranslation += vRight * frameSpeed;
	}
	if (Input::IsKeyDown(KEY_A))
	{
		vTranslation -= vRight * frameSpeed;
	}
	if (Input::IsKeyDown('Q'))
	{
		vTranslation += vUp * frameSpeed;
	}
	if (Input::IsKeyDown('E'))
	{
		vTranslation -= vUp * frameSpeed;
	}

	// check for camera rotation
	static bool sbMouseButtonDown = false;
	bool mouseDown = Input::IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
	if (mouseDown)
	{
		viewMatrix[3] = vTranslation;

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
			mMat = glm::axisAngleMatrix(glm::vec3(0, 1, 0), (float)-iDeltaX / 150.0f);
			vRight = mMat * vRight;
			vUp = mMat * vUp;
			vForward = mMat * vForward;
		}

		viewMatrix[0] = vRight;
		viewMatrix[1] = vUp;
		viewMatrix[2] = vForward;

		GetEntity().GetComponent<TransformComponent>().SetTransform(viewMatrix);
	}
	else
	{
		sbMouseButtonDown = false;
	}
	SetProjectionViewMatrix();
}

const glm::mat4& CameraComponent::GetProjViewMatrix() const
{
	return m_projectionViewMatrix;
}

const glm::mat4& CameraComponent::GetProjMatrix() const
{
	return m_projectionMatrix;
}

const glm::mat4 CameraComponent::GetViewMatrix()
{
	return GetEntity().GetComponent<TransformComponent>().GetTransform();
}

const float CameraComponent::GetCamerAspect(const CameraAspect& cameraAspect)
{
	switch (cameraAspect)
	{
	case CameraAspect::A_4x3: return 4.0f / 3.0f;
	case CameraAspect::A_16x9: return 16.0f / 9.0f;
	case CameraAspect::CurrentWindowSize: return (float)Module::WindowModule::GetWindow()->GetWidth() /
		(float)Module::WindowModule::GetWindow()->GetHeight();

	default: return 1.0f;
	}
}

void CameraComponent::SetProjectionViewMatrix()
{
	m_projectionViewMatrix = m_projectionMatrix * glm::inverse(GetEntity().GetComponent<TransformComponent>().GetTransform());
}
