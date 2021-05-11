#include "ispch.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Serialization/SerializeHelper.h"

#include <glm/gtx/string_cast.hpp>

REGISTER_DEF_TYPE(TransformComponent);

TransformComponent::TransformComponent(EntityManager* entityManager, const EntityID& entity)
	: Component(entityManager, entity)
	, m_transform(glm::mat4(1.0f))
{
	//SetType<TransformComponent>();
	//m_updateEveryFarme = false;
}

TransformComponent::~TransformComponent()
{
}

void TransformComponent::OnUpdate(const float& a_deltaTime)
{
}

//void TransformComponent::OnCreate()
//{
//	__super::OnCreate();
//	IS_REGISTER_PROPERTY(glm::mat4, m_transform, "Transform", ShowInEditor);
//	m_allowRemovale = false;
//}

//void TransformComponent::OnDestroy()
//{
//}

glm::mat4 TransformComponent::GetTransform()
{
	glm::mat4 m = m_transform;

	Entity parentEntity = GetEntity().GetParent();
	if (parentEntity.IsValid())
	{
		while (parentEntity.IsValid())
		{
			m = m + parentEntity.GetComponent<TransformComponent>().GetTransform();
			parentEntity = parentEntity.GetParent();
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
	//m_isDirty = true;
}

//void TransformComponent::Serialize(Serialization::SerializableElement* element, bool force)
//{
//	//element->AddAttribute("UUID", GetUUID());
//	element->AddAttribute("Type", "TransformComponent");
//	//element->AddAttribute("ViewMatrix", SerializeHelper::TypeToString(m_transform));
//}
//
//void TransformComponent::Deserialize(Serialization::SerializableElement* element, bool force)
//{
//	if (auto ptr = element->GetFirstAttribute("ViewMatrix"))
//	{
//		//m_transform = SerializeHelper::StringToType<glm::mat4>(ptr->GetValue());
//	}
//}

