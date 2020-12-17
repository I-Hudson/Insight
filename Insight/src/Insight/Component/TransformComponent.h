#pragma once

#include "Insight/Core.h"
#include "Insight/Component/Component.h"

#include "glm/glm.hpp"

class IS_API TransformComponent : public Component
{
public:
	TransformComponent();
	TransformComponent(SharedPtr<Entity> owner);
	virtual ~TransformComponent() override;

	virtual void OnCreate() override;
	virtual void OnDestroy() override;

	glm::mat4 GetTransform() const;
	void SetTransform(const glm::mat4& mat4);

	const glm::vec3 GetPostion();
	void SetPosition(const glm::vec3& position);

	virtual void Serialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force = false) override;
	virtual void Deserialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force = false) override;

	virtual std::string GetTypeName() const { return "TransformComponent"; }

private:
	glm::mat4 m_transform;

	REGISTER_DEC_TYPE(TransformComponent);
};
