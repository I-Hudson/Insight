#include "ispch.h"
#include "Entity.h"

#include "Insight/Scene/Scene.h"
#include "Insight/Component/Component.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Model/Model.h"
#include "Insight/Instrumentor/Instrumentor.h"

#include "Insight/Log.h"

REGISTER_DEF_TYPE(Entity);

Entity::Entity()
	: Serializable(this, false)
{
	m_name = "Default";
}

Entity::Entity(const std::string& id)
	: Serializable(this, false)
{
	m_name = id;
}

Entity::Entity(const std::string& id, bool attachToScene)
	: Serializable(this, false)
{
	m_name = id;
	m_attachedToScene = false;
}

Entity::~Entity()
{
	RemoveAllComponenets();
}

SharedPtr<Entity> Entity::Create(const std::string& id)
{
	return Insight::Scene::s_CurrentScene->CreateEntity(id);
}

SharedPtr<Entity> Entity::CreateFromModel(SharedPtr<Model> model)
{
	IS_PROFILE_FUNCTION();

	std::string modelName = model->GetName();
	SharedPtr<Entity> e = Entity::Create(modelName);

	unsigned int meshCount = model->GetSubMeshCount();
	if (meshCount > 1)
	{
		for (unsigned int i = 0; i < meshCount; ++i)
		{
			SharedPtr<Mesh> subMesh = model->GetSubMesh(i);
			SharedPtr<Entity> childEntity = Entity::Create(subMesh->GetName());
			childEntity->AddComponent<MeshComponent>()->SetMesh(subMesh);

			e->AddChild(childEntity);
		}
	}
	else if (meshCount == 1)
	{
		e->AddComponent<MeshComponent>()->SetMesh(model->GetSubMesh(0));
	}
	else
	{
		IS_CORE_ERROR("");
	}

	return e;
}

void Entity::Delete()
{
	IS_PROFILE_FUNCTION();

	if (this != nullptr)
	{
		for (int i = 0; i < m_children.size(); ++i)
		{
			Insight::Scene::s_CurrentScene->DeleteEntiy(m_children[i]);
		}
		Insight::Scene::s_CurrentScene->DeleteEntiy(this->shared_from_this());
	}
}

void Entity::OnUpdate(const float deltaTime)
{
	IS_PROFILE_FUNCTION();

	for (auto it = m_components.begin(); it != m_components.end(); ++it)
	{
		(*it)->OnUpdate(deltaTime);
	}
}

void Entity::SetID(const std::string& id)
{
	m_name = id;
}

std::string& Entity::GetID()
{
	return m_name;
}

SharedPtr<Entity> Entity::AddChild(const std::string& childId)
{
	SharedPtr<Entity> e = Insight::Scene::s_CurrentScene->CreateEntity(childId);
	AddChild(e);
	return e;
}

void Entity::AddChild(SharedPtr<Entity> child)
{
	if (std::find(m_children.begin(), m_children.end(), child) == m_children.end())
	{
		m_children.push_back(child);
	}
	child->SetParent(this->shared_from_this());
}

SharedPtr<Entity> Entity::GetChild(int childIndex)
{
	if (childIndex < 0 || childIndex >= m_children.size())
	{
		IS_ASSERT(true, "Entity: GetChild: Out of range.");
		return nullptr;
	}
	return m_children[childIndex];
}

void Entity::RemoveChild(SharedPtr<Entity> child)
{
	auto it = std::find(m_children.begin(), m_children.end(), child);
	if (it != m_children.end())
	{
		m_children.erase(it);
		child->SetParent(nullptr);
	}
}

//void Entity::Serialize(json& out, bool force)
//{
//	if (GetParent() == nullptr || force)
//	{
//		out["UUID"] = GetUUID().c_str();
//		out["Type"] = "Entity";
//		out["Name"] = m_data.Name;
//
//		for (auto it = m_data.Children.begin(); it != m_data.Children.end(); ++it)
//		{
//			(*it)->Serialize(out["Children"][(*it)->GetUUID()], true);
//		}
//
//		for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
//		{
//			(*it)->Serialize(out["Components"][(*it)->GetUUID()]);
//		}
//	}
//}

void Entity::Serialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force)
{
	if (GetParent().expired() || force)
	{
		element->AddString("UUID", GetUUID());
		element->AddString("Type", "Entity");
		element->AddString("Name", m_name);

		for (auto it = m_components.begin(); it != m_components.end(); ++it)
		{
			auto childComponent = element->AddChild("Component");
			(*it)->Serialize(childComponent, true);
		}

		for (auto it = m_children.begin(); it != m_children.end(); ++it)
		{
			auto childEntity = element->AddChild("Entity");
			(*it)->Serialize(childEntity, true);
		}
	}
}

void Entity::Deserialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force)
{
	//if (auto uuid = in->FirstChildElement("UUID"))
	//{
	//	SetUUID(uuid->GetText());
	//}
	//if (auto name = in->FirstChildElement("Name"))
	//{
	//	m_data.Name = name->GetText() != 0 ? name->GetText() : "Defualt";
	//}
	//
	//tinyxml2::XMLNode* children = in->FirstChildElement("Children");
	//if (children != nullptr)
	//{
	//	tinyxml2::XMLNode* child = children->FirstChild();
	//	do
	//	{
	//		Serializable* s = CreateInstanceFromType<Serializable>(child->FirstChildElement("Type")->GetText());
	//		dynamic_cast<Entity*>(s)->SetParent(this);
	//		if (s != nullptr)
	//		{
	//			s->Deserialize(child);
	//			m_data.Children.push_back(dynamic_cast<Entity*>(s));
	//		}
	//
	//		child = child->NextSibling();
	//	} while (child != nullptr);
	//}
	//
	//tinyxml2::XMLNode* components = in->FirstChildElement("Components");
	//if (components != nullptr)
	//{
	//	tinyxml2::XMLNode* c = components->FirstChild();
	//	do
	//	{
	//		Serializable* s = CreateInstanceFromType<Serializable>(c->FirstChildElement("Type")->GetText());
	//		if (s != nullptr)
	//		{
	//			AddComponent(dynamic_cast<Component*>(s));
	//			s->Deserialize(c);
	//		}
	//		c = c->NextSibling();
	//	} while (c != nullptr);
	//}
}

void Entity::AddComponent(SharedPtr<Component> component)
{
	m_components.push_back(component);

	component->SetEntity(this->shared_from_this());
	m_componetBitset[component->m_componentId] = true;

	if (component->m_updateEveryFarme)
	{
		Insight::Scene::ActiveScene()->m_updateComponents.push_back(component);
	}
}

void Entity::RemoveAllComponenets()
{
	IS_PROFILE_FUNCTION();

	for (auto it = m_components.begin(); it != m_components.end(); ++it)
	{
		(*it)->OnDestroy();
	}

	m_components.clear();
	m_componetBitset.reset();
}
