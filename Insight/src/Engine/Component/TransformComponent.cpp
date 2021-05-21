#include "ispch.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Serialization/SerializeHelper.h"

#include <glm/gtx/string_cast.hpp>

REGISTER_DEF_TYPE(TransformComponent);

TransformComponentData::TransformComponentData()
	: Transform(glm::mat4(1.0f))
{
	AllowRemovable = false;
}

TransformComponent::TransformComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity)
	: Component(componentManager, componentID, componentType, entityManager, entity)
{
	SetType<TransformComponent>();
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
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	glm::mat4 m = data.Transform;

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
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	data.Transform = mat4;
}

const glm::vec3 TransformComponent::GetPostion()
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	return data.Transform[3].xyz();
}

void TransformComponent::SetPosition(const glm::vec3& position)
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	data.Transform[3] = glm::vec4(position, 1.0f);
	//m_isDirty = true;
}

glm::vec3 TransformComponent::GetRotation()
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	glm::vec3 rot;
	rot.x = atan2(-data.Transform[1][2], data.Transform[2][2]);
	float c2 = sqrt(pow(data.Transform[0][0], 2) + pow(data.Transform[0][1], 2));
	rot.y = atan2(-data.Transform[0][2], c2);
	float s1 = sin(rot.x);
	float c1 = cos(rot.x);
	rot.z = atan2(s1 * data.Transform[2][0] - c1 * data.Transform[1][0], c1 * data.Transform[1][1] * s1 * -data.Transform[2][1]);
	return rot;
}

void TransformComponent::SetRotation(glm::vec3 rotation)
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	glm::mat4 rotM(1.0f);
	rotM = glm::rotate(rotM, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 translateM = glm::translate(glm::mat4(1.0f), GetPostion());
	data.Transform = translateM * rotM;
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
