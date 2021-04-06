#pragma once

#include "Engine/Component/Component.h"

#include "glm/glm.hpp"

class IS_API TransformComponent : public Component
{
public:
	TransformComponent();
	TransformComponent(Entity* owner);
	virtual ~TransformComponent() override;

	virtual void OnCreate() override;
	virtual void OnDestroy() override;

	glm::mat4 GetTransform() const;
	void SetTransform(const glm::mat4& mat4);

	const glm::vec3 GetPostion();
	void SetPosition(const glm::vec3& position);

	virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
	virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

private:
	glm::mat4 m_transform;

	REGISTER_DEC_TYPE(TransformComponent);
};
