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

const glm::vec3 TransformComponent::GetPostion()
{
	return m_transform[3].xyz();
}

void TransformComponent::SetPosition(const glm::vec3& position)
{
	m_transform[3] = glm::vec4(position, 1.0f);
	m_isDirty = true;
}

void TransformComponent::Serialize(json& out, bool force)
{
	__super::Serialize(out);
	out["Type"] = "TransformComponent";

	out["ViewMatrix"]["X"] = { m_transform[0].x, m_transform[0].y, m_transform[0].z,  m_transform[0].w };
	out["ViewMatrix"]["Y"] = { m_transform[1].x, m_transform[1].y, m_transform[1].z,  m_transform[1].w };
	out["ViewMatrix"]["Z"] = { m_transform[2].x, m_transform[2].y, m_transform[2].z,  m_transform[2].w };
	out["ViewMatrix"]["W"] = { m_transform[3].x, m_transform[3].y, m_transform[3].z,  m_transform[3].w };
}

void TransformComponent::Deserialize(json in, bool force)
{
	__super::Deserialize(in);

	DeserializeTransform(m_transform[0], in["ViewMatrix"]["X"]);
	DeserializeTransform(m_transform[1], in["ViewMatrix"]["Y"]);
	DeserializeTransform(m_transform[2], in["ViewMatrix"]["Z"]);
	DeserializeTransform(m_transform[3], in["ViewMatrix"]["W"]);
}

void TransformComponent::DeserializeTransform(glm::vec4& vector, json data)
{
	int i = 0;
	float values[4];

	for (auto it = data.begin(); it != data.end(); ++it)
	{
		values[i] = (*it).get<float>();
		++i;
	}

	vector.x = values[0];
	vector.y = values[1];
	vector.z = values[2];
	vector.w = values[3];
}

