#include "ispch.h"
#include "TransformComponent.h"
#include "Insight/Entitiy/Entity.h"

#include <glm/gtx/string_cast.hpp>

REGISTER_DEF_TYPE(TransformComponent);

TransformComponent::TransformComponent()
	: Component(nullptr, ComponentType::TRANSFORM)
	, m_transform(glm::mat4(1.0f))
{
}

TransformComponent::TransformComponent(Entity* owner)
	: Component(owner, ComponentType::TRANSFORM)
	, m_transform(glm::mat4(1.0f))
{
}

TransformComponent::~TransformComponent()
{
}

glm::mat4 TransformComponent::GetTransform() const
{
	glm::mat4 m = m_transform;

	const Entity* parent = GetEntity()->GetParent();
	while (parent != nullptr)
	{
		m = m + parent->GetComponent<TransformComponent>()->GetTransform();
		parent = parent->GetParent();
	}

	return m;
}

const glm::vec3 TransformComponent::GetPostion()
{
	return m_transform[3].xyz();
}

void TransformComponent::SetPosition(const glm::vec3& position)
{
	m_transform[3] = glm::vec4(position, 1.0f);
}

void TransformComponent::Serialize(json& out)
{
	out["UUID"] = GetUUID();
	out["Type"] = "TransformComponent";
	out["ViewMatrix"]["X"] = { m_transform[0].x, m_transform[0].y, m_transform[0].z };
	out["ViewMatrix"]["Y"] = { m_transform[1].x, m_transform[1].y, m_transform[1].z };
	out["ViewMatrix"]["Z"] = { m_transform[2].x, m_transform[2].y, m_transform[2].z };
	out["ViewMatrix"]["W"] = { m_transform[3].x, m_transform[3].y, m_transform[3].z };
}

void TransformComponent::Deserialize(json in)
{
}

