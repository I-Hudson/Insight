#include "ispch.h"
#include "Entity.h"

#include "Insight/Scene/Scene.h"
#include "Insight/Component/Component.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Assimp/Model.h"
#include "Insight/Instrumentor/Instrumentor.h"

#include "Insight/Log.h"

REGISTER_DEF_TYPE(Entity);

Entity::Entity()
	: Insight::UUID()
	, Serializable(this, false)
{
	m_data.Name = "Default";
	Insight::Scene::s_CurrentScene->m_registry.push_back(this);
}

Entity::Entity(const std::string& id)
	: Insight::UUID()
	, Serializable(this, false)
{
	m_data.Name = id;
	Insight::Scene::s_CurrentScene->m_registry.push_back(this);
}

Entity::~Entity()
{
	RemoveAllComponenets();
}

Entity* Entity::Create(const std::string& id)
{
	return Insight::Scene::s_CurrentScene->CreateEntity(id);
}

Entity* Entity::CreateFromModel(Model* model)
{
	IS_PROFILE_FUNCTION();

	std::string modelName = model->GetName();
	Entity* e = Entity::Create(modelName);

	unsigned int meshCount = model->GetSubMeshCount();
	for (unsigned int i = 0; i < meshCount; ++i)
	{
		Mesh* subMesh = model->GetSubMesh(i);
		Entity* childEntity = Entity::Create(subMesh->GetName());
		childEntity->AddComponent<MeshComponent>()->SetMesh(subMesh);

		e->AddChild(childEntity);
	}

	return e;
}

void Entity::Delete()
{
	IS_PROFILE_FUNCTION();

	if (this != nullptr)
	{
		for (int i = 0; i < m_data.Children.size(); ++i)
		{
			Insight::Scene::s_CurrentScene->DeleteEntiy(m_data.Children[i]);
		}
		Insight::Scene::s_CurrentScene->DeleteEntiy(this);
	}
}

void Entity::OnUpdate(const float deltaTime)
{
	IS_PROFILE_FUNCTION();

	for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
	{
		(*it)->OnUpdate(deltaTime);
	}
}

void Entity::SetID(const std::string& id)
{
	m_data.Name = id;
}

const std::string& Entity::GetID() const
{
	return m_data.Name;
}

Entity* Entity::AddChild(const std::string& childId)
{
	Entity* e = Insight::Scene::s_CurrentScene->CreateEntity(childId);
	AddChild(e);
	return e;
}

void Entity::AddChild(Entity* child)
{
	if (std::find(m_data.Children.begin(), m_data.Children.end(), child) == m_data.Children.end())
	{
		m_data.Children.push_back(child);
	}
	child->SetParent(this);
}

Entity* Entity::GetChild(int childIndex)
{
	if (childIndex < 0 || childIndex >= m_data.Children.size())
	{
		IS_ASSERT(true, "Entity: GetChild: Out of range.");
		return nullptr;
	}
	return m_data.Children[childIndex];
}

void Entity::RemoveChild(Entity* child)
{
	auto it = std::find(m_data.Children.begin(), m_data.Children.end(), child);
	if (it != m_data.Children.end())
	{
		m_data.Children.erase(it);
		child->SetParent(nullptr);
	}
}

void Entity::Serialize(json& out, bool force)
{
	if (GetParent() == nullptr || force)
	{
		out["UUID"] = GetUUID().c_str();
		out["Type"] = "Entity";
		out["Name"] = m_data.Name;

		for (auto it = m_data.Children.begin(); it != m_data.Children.end(); ++it)
		{
			(*it)->Serialize(out["Children"][(*it)->GetUUID()], true);
		}

		for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
		{
			(*it)->Serialize(out["Components"][(*it)->GetUUID()]);
		}
	}
}

void Entity::Deserialize(json in, bool force)
{
	SetUUID(in["UUID"]);
	m_data.Name = in["Name"];

	for (auto it = in["Children"].begin(); it != in["Children"].end(); ++it)
	{
		Serializable* s = CreateInstanceFromType<Serializable>((*it)["Type"]);
		dynamic_cast<Entity*>(s)->SetParent(this);
		if (s != nullptr)
		{
			s->Deserialize(*it);
			m_data.Children.push_back(dynamic_cast<Entity*>(s));
		}
	}

	for (auto it = in["Components"].begin(); it != in["Components"].end(); ++it)
	{
		Serializable* s = CreateInstanceFromType<Serializable>((*it)["Type"]);
		AddComponent(static_cast<Component*>(s));

		if (s != nullptr)
		{
			s->Deserialize(*it);
		}
	}
}

void Entity::AddComponent(Component* component)
{
	m_data.Components.push_back(component);

	component->SetEntity(this);
	m_data.ComponetBitset[component->m_componentId] = true;

	if (component->m_updateEveryFarme)
	{
		Insight::Scene::ActiveScene()->m_updateComponents.push_back(component);
	}
}

void Entity::RemoveAllComponenets()
{
	IS_PROFILE_FUNCTION();

	for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
	{
		DELETE_ON_HEAP((*it));
	}
	m_data.Components.clear();
	m_data.ComponetBitset.reset();
}
