#pragma once

#include "Insight/Core.h"
#include "Insight/Component/Component.h"

#include <../vendor/glm/glm/glm.hpp>

class IS_API TransformComponent : public Component
{
public:
	TransformComponent();
	TransformComponent(Entity* owner);
	virtual ~TransformComponent() override;

	glm::mat4 GetTransform() const;
	const glm::vec3 GetPostion();
	void SetPosition(const glm::vec3& position);

	virtual void Serialize(json& out, bool force = false) override;
	virtual void Deserialize(json in, bool force = false) override;

private:
	void DeserializeTransform(glm::vec4& vector, json data);

	glm::mat4 m_transform;

	REGISTER_DEC_TYPE(TransformComponent);
};
