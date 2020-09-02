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

	tinyxml2::XMLElement* ViewMatrixX = doc->NewElement("ViewMatrixX");
	ViewMatrixX->SetAttribute("x", m_transform[0].x); ViewMatrixX->SetAttribute("y", m_transform[0].y); ViewMatrixX->SetAttribute("z", m_transform[0].z); ViewMatrixX->SetAttribute("w", m_transform[0].w);
	out->InsertEndChild(ViewMatrixX);

	tinyxml2::XMLElement* ViewMatrixY = doc->NewElement("ViewMatrixY");
	ViewMatrixY->SetAttribute("x", m_transform[1].x); ViewMatrixY->SetAttribute("y", m_transform[1].y); ViewMatrixY->SetAttribute("z", m_transform[1].z); ViewMatrixY->SetAttribute("w", m_transform[1].w);
	out->InsertEndChild(ViewMatrixY);

	tinyxml2::XMLElement* ViewMatrixZ = doc->NewElement("ViewMatrixZ");
	ViewMatrixZ->SetAttribute("x", m_transform[2].x); ViewMatrixZ->SetAttribute("y", m_transform[2].y); ViewMatrixZ->SetAttribute("z", m_transform[2].z); ViewMatrixZ->SetAttribute("w", m_transform[2].w);
	out->InsertEndChild(ViewMatrixZ);

	tinyxml2::XMLElement* ViewMatrixW = doc->NewElement("ViewMatrixW");
	ViewMatrixW->SetAttribute("x", m_transform[3].x); ViewMatrixW->SetAttribute("y", m_transform[3].y); ViewMatrixW->SetAttribute("z", m_transform[3].z); ViewMatrixW->SetAttribute("w", m_transform[3].w);
	out->InsertEndChild(ViewMatrixW);

}

void TransformComponent::Deserialize(tinyxml2::XMLNode* in, bool force)
{
	DeserializeTransform(m_transform[0], in->FirstChildElement("ViewMatrixX"));
	DeserializeTransform(m_transform[1], in->FirstChildElement("ViewMatrixY"));
	DeserializeTransform(m_transform[2], in->FirstChildElement("ViewMatrixZ"));
	DeserializeTransform(m_transform[3], in->FirstChildElement("ViewMatrixW"));
}

void TransformComponent::DeserializeTransform(glm::vec4& vector, tinyxml2::XMLElement* data)
{
	vector.x = data->FloatAttribute("x");
	vector.y = data->FloatAttribute("y");
	vector.z = data->FloatAttribute("z");
	vector.w = data->FloatAttribute("w");
}

