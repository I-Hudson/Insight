#include "ispch.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Serialization/SerializeHelper.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Core/Maths/Math.h"

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
	glm::vec3 pos = position;
	//if (::Graphics::IsVulkan())
	//{
	//	pos.y *= -1;
	//}
	data.Transform[3] = glm::vec4(pos, 1.0f);
	//m_isDirty = true;
}

glm::vec3 TransformComponent::GetRotation()
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	glm::vec3 rot;
	rot.x = atan2(-data.Transform[1][2], data.Transform[2][2]);
	float c2 = (float)sqrt(pow(data.Transform[0][0], 2) + pow(data.Transform[0][1], 2));
	rot.y = atan2(-data.Transform[0][2], c2);
	float s1 = sin(rot.x);
	float c1 = cos(rot.x);
	rot.z = atan2(s1 * data.Transform[2][0] - c1 * data.Transform[1][0], c1 * data.Transform[1][1] * s1 * -data.Transform[2][1]);
	return rot;
}

void TransformComponent::SetRotation(glm::vec3 rotation)
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	glm::mat4 rotM = Insight::Maths::RotationVectorRadToMatrix(rotation);
	glm::mat4 translateM = glm::translate(glm::mat4(1.0f), GetPostion());
	glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), GetScale());
	data.Transform = translateM * rotM * scaleM;
}

glm::vec3 TransformComponent::GetScale()
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	return glm::vec3(data.Transform[0][0], data.Transform[1][1], data.Transform[2][2]);
}

void TransformComponent::SetScale(glm::vec3 scale)
{
	TransformComponentData& data = GetComponentData<TransformComponentData>();
	glm::mat4 scaleM(1.0f);
	scaleM = glm::scale(scaleM, scale);
	glm::mat4 translateM = glm::translate(glm::mat4(1.0f), GetPostion());
	glm::mat4 rotM = Insight::Maths::RotationVectorRadToMatrix(GetRotation());
	data.Transform = translateM * rotM * scaleM;
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
