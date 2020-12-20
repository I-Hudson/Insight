#include "ispch.h"
#include "TransformComponent.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/Serialization/SerializeHelper.h"

#include <glm/gtx/string_cast.hpp>

REGISTER_DEF_TYPE(TransformComponent);

TransformComponent::TransformComponent()
	: Component(nullptr)
	, m_transform(glm::mat4(1.0f))
{
	m_updateEveryFarme = false;
	m_componentId = GetComponentID<TransformComponent>();
}

TransformComponent::TransformComponent(SharedPtr<Entity> owner)
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
	IS_REGISTER_PROPERTY(glm::mat4, m_transform, "Transform", ShowInEditor);
}

void TransformComponent::OnDestroy()
{
}

glm::mat4 TransformComponent::GetTransform() const
{
	glm::mat4 m = m_transform;

	if (SharedPtr<Entity> parentPtr = GetEntity().lock()->GetParent().lock())
	{
		SharedPtr<Entity> parent = parentPtr;
		while (parent != nullptr)
		{
			m = m + parent->GetComponent<TransformComponent>()->GetTransform();
			parent = parent->GetParent().lock();
		}
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

void TransformComponent::Serialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force)
{
	element->AddAttribute("UUID", GetUUID());
	element->AddAttribute("Type", "TransformComponent");
	element->AddAttribute("ViewMatrix", SerializeHelper::TypeToString(m_transform));
}

void TransformComponent::Deserialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force)
{
	if (auto ptr = element->GetFirstAttribute("ViewMatrix"))
	{
		m_transform = SerializeHelper::StringToType<glm::mat4>(ptr->GetValue());
	}
}

