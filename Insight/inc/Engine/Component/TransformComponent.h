#pragma once

#include "Engine/Component/Component.h"
#include "glm/glm.hpp"

struct TransformComponentData : public ComponentData
{
	TransformComponentData();
	glm::mat4 Transform;
};

class IS_API TransformComponent : public Component
{
public:
	TransformComponent() { }
	TransformComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity);
	virtual ~TransformComponent() override;

	virtual void OnUpdate(const float& a_deltaTime) override;
	//virtual void OnCreate() override;
	//virtual void OnDestroy() override;

	glm::mat4 GetTransform();
	void SetTransform(const glm::mat4& mat4);

	const glm::vec3 GetPostion();
	void SetPosition(const glm::vec3& position);

	//virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
	//virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

private:

	REGISTER_DEC_TYPE(TransformComponent);
};
