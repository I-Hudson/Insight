#include "Engine/Component/TransformComponent.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Serialization/SerializeHelper.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Core/Maths/Math.h"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>

REGISTER_DEF_TYPE(TransformComponent);

TransformComponentData::TransformComponentData()
	: Position(glm::vec3(0)), Rotation(glm::vec3(0)), Scale(glm::vec3(1))
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
	glm::mat4 m = glm::mat4(1);
	m = glm::translate(m, data.Position);
	m *= glm::toMat4(data.Rotation);
	m = glm::scale(m, data.Scale);

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
	glm::vec3 position, scale;
	glm::vec3 rotation;
	Insight::Maths::DecomposeTranfrom(mat4, position, rotation, scale);

	TransformComponentData& data = GetComponentData<TransformComponentData>();
	data.Position = position;
	data.Rotation = rotation;
	data.Scale = scale;
}

const glm::vec3 TransformComponent::GetPostion()
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	return data.Position;
}

void TransformComponent::SetPosition(const glm::vec3& position)
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	glm::vec3 pos = position;
	data.Position = pos;
}

glm::vec3 TransformComponent::GetRotation()
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	return glm::eulerAngles(data.Rotation);
}

void TransformComponent::SetRotation(glm::vec3 rotation)
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	data.Rotation = glm::quat(glm::vec3(rotation.x, rotation.y, rotation.z));
}

glm::vec3 TransformComponent::GetScale()
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	return data.Scale;
}

void TransformComponent::SetScale(glm::vec3 scale)
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	data.Scale = scale;
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
