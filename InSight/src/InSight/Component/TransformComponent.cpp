#include "ispch.h"
#include "TransformComponent.h"

TransformComponent::TransformComponent(Entity* owner)
	: Component(owner, ComponentType::TRANSFORM), m_transform(glm::mat4(0))
{
}

TransformComponent::~TransformComponent()
{
}

const glm::mat4& TransformComponent::GetTransform() const
{
	return m_transform;
}

const glm::vec3 TransformComponent::GetPostion()
{
	return m_transform[3].xyz();
}

void TransformComponent::SetPosition(const glm::vec3& position)
{
	m_transform[3] = glm::vec4(position, 1.0f);
}

