#include "ispch.h"
#include "TransformComponent.h"
#include "Insight/Entitiy/Entity.h"

#include <glm/gtx/string_cast.hpp>

REGISTER_DEF_TYPE(TransformComponent);

TransformComponent::TransformComponent()
	: Component(nullptr)
	, m_transform(glm::mat4(1.0f))
{
	m_updateEveryFarme = false;
	m_componentId = GetComponentID<TransformComponent>();
}

TransformComponent::TransformComponent(Entity* owner)
	: Component(owner)
	, m_transform(glm::mat4(1.0f))
{
	m_updateEveryFarme = false;
	m_componentId = GetComponentID<TransformComponent>();
}

TransformComponent::~TransformComponent()
{
}

void TransformComponent::OnCreate()
{
	__super::OnCreate();
	IS_REGISTER_PROPERTY(glm::mat4, m_transform, "Transform", UIFlags_ShowInEditor);
}

void TransformComponent::OnDestroy()
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

void TransformComponent::SetTransform(const glm::mat4& mat4)
{
	m_transform = mat4;
}

const glm::vec3 TransformComponent::GetPostion()
{
	return m_transform[3].xyz();
}

void TransformComponent::SetPosition(const glm::vec3& position)
{
	m_transform[3] = glm::vec4(position, 1.0f);
	m_isDirty = true;
}

void TransformComponent::Serialize(tinyxml2::XMLNode* out, tinyxml2::XMLDocument* doc, bool force)
{
	tinyxml2::XMLElement* Type = doc->NewElement("Type");
	Type->SetText("TransformComponent");
	out->InsertEndChild(Type);

	SerializeHelper::SerializeMat4(out, doc, "ViewMatrix", m_transform);
}

void TransformComponent::Deserialize(tinyxml2::XMLNode* in, bool force)
{
	m_transform = SerializeHelper::DeserializeMat4(in, "ViewMatrix");
}

