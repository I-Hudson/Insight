#pragma once

#include "Insight/Core.h"
#include "Insight/Component/Component.h"

#include "glm/glm.hpp"

class IS_API TransformComponent : public Component
{
public:
	TransformComponent();
	TransformComponent(Entity* owner);
	virtual ~TransformComponent() override;

	virtual void OnCreate() override;

	glm::mat4 GetTransform() const;
	void SetTransform(const glm::mat4& mat4);

	const glm::vec3 GetPostion();
	void SetPosition(const glm::vec3& position);

	virtual void Serialize(tinyxml2::XMLNode* out, tinyxml2::XMLDocument* doc, bool force = false) override;
	virtual void Deserialize(tinyxml2::XMLNode* in, bool force = false) override;

private:
	glm::mat4 m_transform;

	REGISTER_DEC_TYPE(TransformComponent);
};
