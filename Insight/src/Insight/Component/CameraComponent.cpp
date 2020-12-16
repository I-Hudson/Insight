#include "ispch.h"
#include "CameraComponent.h"

#include "Insight/Component/TransformComponent.h"
#include "Insight/Input/Input.h"
#include "Insight/Module/WindowModule.h"
#include "Insight/Module/GraphicsModule.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

#include "Insight/Editor/Components/CameraComponentEditorDrawer.h"
#include "Insight/RTTI/RTTI.h"

REGISTER_DEF_TYPE(CameraComponent);

CameraComponent::CameraComponent()
	: Component(nullptr)
{
	m_updateEveryFarme = true;
	m_componentId = GetComponentID<CameraComponent>();
}

CameraComponent::CameraComponent(SharedPtr<Entity> owner)
	: Component(owner)
{
	m_updateEveryFarme = true;
	m_componentId = GetComponentID<CameraComponent>();

	CUSTOM_EDITOR_DRAWER(Insight::Editor::CameraComponentEditorDrawer, CameraComponent);
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::OnCreate()
{
	__super::OnCreate();

	IS_PROPERTY(float, m_fov, "FOV", ShowInEditor | ReadOnly);
	IS_PROPERTY(float, m_nearPlane, "Near Plane", ShowInEditor | ClampZero);
	IS_PROPERTY(float, m_farPlane, "Far Plane", ShowInEditor | ReadOnly);

	m_cameraSpeed = 50;
	SetProjMatrix(90, CameraAspect::CurrentWindowSize, 0.1f, 100000.0f);
	SetViewMatrix(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

	if (!Insight::Module::GraphicsModule::Instance()->HasMainCamera())
	{
		Insight::Module::GraphicsModule::Instance()->SetMainCamera(this);
	}
}

void CameraComponent::OnDestroy()
{
	if (Insight::Module::GraphicsModule::IsInitialised() && Insight::Module::GraphicsModule::Instance()->IsThisMainCamera(this))
	{
		Insight::Module::GraphicsModule::Instance()->SetMainCamera(nullptr);
	}
}

void CameraComponent::Serialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force)
{
	//tinyxml2::XMLElement* Type = doc->NewElement("Type"); // GetNewElement();
	//Type->SetText("CameraComponent");
	//data->InsertEndChild(Type);
	//
	//SerializeHelper::SerializeFloat(data, doc, "FOV", m_fov);
	//SerializeHelper::SerializeFloat(data, doc, "NearPlane", m_nearPlane);
	//SerializeHelper::SerializeFloat(data, doc, "FarPlane", m_farPlane);
}

void CameraComponent::Deserialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force)
{
	//m_fov = data->FirstChildElement("FOV")->FloatText();
	//m_nearPlane = data->FirstChildElement("NearPlane")->FloatText();
	//m_farPlane = data->FirstChildElement("FarPlane")->FloatText();
}

void CameraComponent::SetViewMatrix(const glm::mat4& a_value)
{
	if (SharedPtr<Entity> parentPtr = GetEntity().lock())
	{
		parentPtr->GetComponent<TransformComponent>()->SetTransform(a_value);
		SetProjectionViewMatrix();
	}
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


	glm::mat4 viewMatrix = glm::mat4(1.0f);
	if (SharedPtr<Entity> parentPtr = GetEntity().lock())
	{
		viewMatrix = parentPtr->GetComponent<TransformComponent>()->GetTransform();
	}

	// Get the camera's forward, right, up, and location vectors
	glm::vec4 vForward = viewMatrix[2];
	glm::vec4 vRight = viewMatrix[0];
	glm::vec4 vUp = viewMatrix[1];
	glm::vec4 vTranslation = viewMatrix[3];

	float frameSpeed = Input::KeyDown(KEY_LEFT_SHIFT) ? a_deltaTime * m_cameraSpeed * 2 : a_deltaTime * m_cameraSpeed;

	// Translate camera
	if (Input::KeyDown(KEY_W))
	{
		vTranslation -= vForward * frameSpeed;
	}
	if (Input::KeyDown(KEY_S))
	{
		vTranslation += vForward * frameSpeed;
	}
	if (Input::KeyDown(KEY_D))
	{
		vTranslation += vRight * frameSpeed;
	}
	if (Input::KeyDown(KEY_A))
	{
		vTranslation -= vRight * frameSpeed;
	}
	if (Input::KeyDown('Q'))
	{
		vTranslation += vUp * frameSpeed;
	}
	if (Input::KeyDown('E'))
	{
		vTranslation -= vUp * frameSpeed;
	}

	// check for camera rotation
	static bool sbMouseButtonDown = false;
	bool mouseDown = Input::MouseButtonDown(MOUSE_BUTTON_RIGHT);
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

		if (SharedPtr<Entity> parentPtr = GetEntity().lock())
		{
			parentPtr->GetComponent<TransformComponent>()->SetTransform(viewMatrix);
		}
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

const glm::mat4 CameraComponent::GetViewMatrix() const
{
	if (SharedPtr<Entity> parentPtr = GetEntity().lock())
	{
		return parentPtr->GetComponent<TransformComponent>()->GetTransform();
	}
	return glm::mat4(1.0f);
}

const float CameraComponent::GetCamerAspect(const CameraAspect& cameraAspect)
{
	switch (cameraAspect)
	{
	case CameraAspect::A_4x3: return 4.0f / 3.0f;
	case CameraAspect::A_16x9: return 16.0f / 9.0f;
	case CameraAspect::CurrentWindowSize: return (float)Insight::Module::WindowModule::GetWindow()->GetWidth() /
		(float)Insight::Module::WindowModule::GetWindow()->GetHeight();

	default: return 1.0f;
	}
}

void CameraComponent::SetProjectionViewMatrix()
{
	if (SharedPtr<Entity> parentPtr = GetEntity().lock())
	{
		m_projectionViewMatrix = m_projectionMatrix * glm::inverse(parentPtr->GetComponent<TransformComponent>()->GetTransform());
	}
}
