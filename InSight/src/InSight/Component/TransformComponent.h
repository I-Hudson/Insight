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

	virtual void Serialize(json& out) override;
	virtual void Deserialize(json in) override;

private:
	glm::mat4 m_transform;

	REGISTER_DEC_TYPE(TransformComponent);
};
