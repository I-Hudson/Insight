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

	//Entity* e = GetEntity();
	//for (size_t i = 0; i < e->GetChildCount(); ++i)
	//{
	//	Entity* c = e->GetChild(i);
	//	if (c->HasComponent<TransformComponent>())
	//	{
	//		glm::vec3 cPos = c->GetComponent<TransformComponent>()->GetPostion();
	//		c->GetComponent<TransformComponent>()->SetPosition(position);
	//		cPos = c->GetComponent<TransformComponent>()->GetPostion();
	//
	//		IS_INFO("");
	//	}
	//}
}

void TransformComponent::Serialize(std::ostream& out)
{
	out << "TransformComponent: " << GetUUID() ENDL;
	out << "ViewMatrix:" << glm::to_string(m_transform) ENDL;
}

void TransformComponent::Deserialize(std::istream& in)
{
}

