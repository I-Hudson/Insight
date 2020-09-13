#pragma once
#include "Insight/Core.h"
#include "Component.h"
#include <glm/glm.hpp>

class Entity;

enum CameraAspect
{
	A_4x3,
	A_16x9,

	CurrentWindowSize
};

class IS_API CameraComponent : public Component
{
public:
	CameraComponent();
	CameraComponent(Entity* owner);
	virtual ~CameraComponent() override;

	virtual void OnCreate() override;


	virtual void Serialize(tinyxml2::XMLNode* data, tinyxml2::XMLDocument* doc, bool force = false) override;
	virtual void Deserialize(tinyxml2::XMLNode* data, bool force = false) override;

	//Set the view matrix for the camera
	void SetViewMatrix(const glm::mat4& a_value);
	//Set the projection matrix for the camera
	void SetProjMatrix(const float& a_fov, const CameraAspect& a_aspect, const float& a_near, const float& a_far);

	//Set the position for the camera
	void SetPosition(const glm::vec3& a_value);
	void SetFov(const float& fov);
	const float GetFov() { return m_fov; }
	virtual void OnUpdate(const float& a_deltaTime) override;

	//Get the projection view matrix
	const glm::mat4& GetProjViewMatrix() const;
	const glm::mat4& GetProjMatrix() const;
	const glm::mat4 GetViewMatrix() const;

	const float GetCamerAspect(const CameraAspect& cameraAspect);

private:
	void SetProjectionViewMatrix();

	CameraAspect m_cameraAspect;
	float m_fov;
	float m_nearPlane;
	float m_farPlane;

	glm::mat4 m_projectionViewMatrix;
	glm::mat4 m_projectionMatrix;

	REGISTER_DEC_TYPE(CameraComponent);
};

