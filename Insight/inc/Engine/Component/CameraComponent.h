#pragma once

#include "Engine/Component/Component.h"
#include <glm/glm.hpp>

enum CameraAspect
{
	A_1,
	A_4x3,
	A_16x9,

	CurrentWindowSize
};

class IS_API CameraComponent : public Component
{
public:
	CameraComponent() { }
	CameraComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity);
	virtual ~CameraComponent() override;

	virtual void OnUpdate(const float& a_deltaTime) override;

	//virtual void OnCreate() override;
	//virtual void OnDestroy() override;

	//virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
	//virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

	//Set the view matrix for the camera
	void SetViewMatrix(const glm::mat4& a_value);
	//Set the projection matrix for the camera
	void SetProjMatrix(const float& a_fov, const CameraAspect& a_aspect, const float& a_near, const float& a_far);
	void SetProjMatrix(const float& a_fov, const float& a_near, const float& a_far);
	void SetOthroMatrix(const float& left, const float& right, const float& bottom, const float& top, const float& a_near, const float& a_far);

	//Set the position for the camera
	void SetPosition(const glm::vec3& a_value);
	void SetFov(const float& fov);
	void SetCameraSpeed(const float& cameraSpeed);

	const float& GetFov() const { return m_fov; }
	const float& GetNearPlane() const { return m_nearPlane; }
	const float& GetFarPlane() const { return m_farPlane; }
	const float& GetCameraSpeed() const { return m_cameraSpeed; }

	//Get the projection view matrix
	const glm::mat4& GetProjViewMatrix() const;
	const glm::mat4& GetProjMatrix() const;
	const glm::mat4 GetViewMatrix();

	const float GetCamerAspect(const CameraAspect& cameraAspect);

private:
	void SetProjectionViewMatrix();

	CameraAspect m_cameraAspect;

	float m_fov;
	float m_nearPlane;
	float m_farPlane;
	float m_cameraSpeed;

	glm::mat4 m_projectionViewMatrix;
	glm::mat4 m_projectionMatrix;

	REGISTER_DEC_TYPE(CameraComponent);
};

