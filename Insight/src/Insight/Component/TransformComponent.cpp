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

//void TransformComponent::Serialize(json& out, bool force)
//{
//	__super::Serialize(out);
//	out["Type"] = "TransformComponent";
//
//	out["ViewMatrix"]["X"] = { m_transform[0].x, m_transform[0].y, m_transform[0].z,  m_transform[0].w };
//	out["ViewMatrix"]["Y"] = { m_transform[1].x, m_transform[1].y, m_transform[1].z,  m_transform[1].w };
//	out["ViewMatrix"]["Z"] = { m_transform[2].x, m_transform[2].y, m_transform[2].z,  m_transform[2].w };
//	out["ViewMatrix"]["W"] = { m_transform[3].x, m_transform[3].y, m_transform[3].z,  m_transform[3].w };
//}

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

