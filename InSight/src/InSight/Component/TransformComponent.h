#pragma once

#include "Insight/Core.h"
#include "Insight/Component/Component.h"

#include <../vendor/glm/glm/glm.hpp>

class IS_API TransformComponent : public Component
{
public:
	TransformComponent(Entity* owner);
	virtual ~TransformComponent() override;

	const glm::mat4& GetTransform() const;
	const glm::vec3 GetPostion();
	void SetPosition(const glm::vec3& position);

private:
	glm::mat4 m_transform;
};
