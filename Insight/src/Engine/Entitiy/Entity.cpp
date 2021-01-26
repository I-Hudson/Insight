#include "ispch.h"
#include "Entity.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Component/Component.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Model/Model.h"
#include "Engine/Instrumentor/Instrumentor.h"

#include "Engine/Core/Log.h"

REGISTER_DEF_TYPE(Entity);

Entity::Entity()
	: Serializable(this, false)
	, m_parent(nullptr)
{
	m_name = "Default";
}

Entity::Entity(const std::string& id)
	: Serializable(this, false)
	, m_parent(nullptr)
{
	m_name = id;
}

Entity::Entity(const std::string& id, bool attachToScene)
	: Serializable(this, false)
	, m_parent(nullptr)
{
	m_name = id;
	m_attachedToScene = false;
}

Entity::~Entity()
{
	RemoveAllComponenets();
}

Entity* Entity::New(const std::string& id)
{
	return Scene::s_CurrentScene->CreateEntity(id);
}

Entity* Entity::CreateFromModel(Model* model)
{
	IS_PROFILE_FUNCTION();

	std::string modelName = model->GetModelName();
	Entity* e = Entity::New(modelName);
	e->AddComponent<MeshComponent>()->SetMesh(model->GetMesh());


	return e;
}

void Entity::Delete()
{
	IS_PROFILE_FUNCTION();

	if (this != nullptr)
	{
		for (int i = 0; i < m_children.size(); ++i)
		{
			Scene::s_CurrentScene->DeleteEntiy(m_children[i]);
		}
		Scene::s_CurrentScene->DeleteEntiy(this);
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

Entity* Entity::AddChild(const std::string& childId)
{
	Entity* e = Scene::s_CurrentScene->CreateEntity(childId);
	AddChild(e);
	return e;
}

void Entity::AddChild(Entity* child)
{
	if (std::find(m_children.begin(), m_children.end(), child) == m_children.end())
	{
		m_children.push_back(child);
	}
	child->SetParent(this);
}

Entity* Entity::GetChild(int childIndex)
{
	if (childIndex < 0 || childIndex >= m_children.size())
	{
		IS_ASSERT(true, "Entity: GetChild: Out of range.");
		return nullptr;
	}
	return m_children[childIndex];
}

void Entity::RemoveChild(Entity* child)
{
	auto it = std::find(m_children.begin(), m_children.end(), child);
	if (it != m_children.end())
	{
		m_children.erase(it);
		child->SetParent(nullptr);
	}
}

void Entity::Serialize(Serialization::SerializableElement* element, bool force)
{
	if (GetParent() || force)
	{
		element->AddAttribute("UUID", GetUUID());
		element->AddAttribute("Type", "Entity");
		element->AddAttribute("Name", m_name);

		auto componentsRoot = element->AddChild("Components");
		for (auto it = m_components.begin(); it != m_components.end(); ++it)
		{
			auto childComponent = componentsRoot->AddChild("Component");
			(*it)->Serialize(childComponent, true);
		}

		auto childrenRoot = element->AddChild("Children");
		for (auto it = m_children.begin(); it != m_children.end(); ++it)
		{
			auto childEntity = childrenRoot->AddChild("Entity");
			(*it)->Serialize(childEntity, true);
		}
	}
}

void Entity::Deserialize(Serialization::SerializableElement* element, bool force)
{
	if (auto ptr = element->GetFirstAttribute("UUID"))
	{
		SetUUID(ptr->GetValue());
	}
	else
	{
		IS_CORE_ERROR("[Entity::Deserialize] UUID not found.");
	}

	if (auto ptr = element->GetFirstAttribute("Name"))
	{
		m_name = ptr->GetValue();
	}
	else
	{
		IS_CORE_ERROR("[Entity::Deserialize] Name not found.");
	}

	auto componentRoot = element->GetFirstChild("Components");
	for (auto component : componentRoot->GetAllChildren())
	{
		if (auto ptr = component->GetFirstAttribute("Type"))
		{
			if (Component* componentObject = Serialization::Serializable::NewInstanceFromType<Component>(ptr->GetValue()))
			{
				AddComponent(componentObject);
				componentObject->Deserialize(component);
			}
			IS_CORE_ERROR("[Entity::Deserialize] Child entity is null.");
		}
	} 

	auto childrenRoot = element->GetFirstChild("Children");
	for (auto child : childrenRoot->GetAllChildren())
	{
		if (auto ptr = child->GetFirstAttribute("Type"))
		{
			if (Entity* childObject = NewInstanceFromType<Entity>(ptr->GetValue()))
			{
				childObject->Deserialize(child);
				m_children.push_back(childObject);
			}
			IS_CORE_ERROR("[Entity::Deserialize] Child entity is null.");
		}
	}
}

bool Entity::HasComponent(const Type& type)
{
	return m_componetBitset[GetComponentID(type)];
}

void Entity::AddComponent(Component* component)
{
	m_components.push_back(component);

	component->SetEntity(this);
	m_componetBitset[component->m_componentId] = true;

	if (component->m_updateEveryFarme)
	{
		Scene::ActiveScene()->m_updateComponents.push_back(component);
	}
}

void Entity::RemoveComponent(const std::string& uuid)
{
	auto it = std::find_if(m_components.begin(), m_components.end(), [uuid](Component* ptr)
		{
			return ptr->GetUUID() == uuid;
		});

	if (it != m_components.end())
	{
		(*it)->OnDestroy();

		m_componetBitset[GetComponentID((*it)->GetType())] = false;

		if (m_attachedToScene)
		{
			auto sceneUpdateComponent = std::find_if(Scene::ActiveScene()->m_updateComponents.begin(),
				Scene::ActiveScene()->m_updateComponents.end(), [it](Component* ptr)
				{
					return (*it)->GetUUID() == ptr->GetUUID();
				});
			if (sceneUpdateComponent != Scene::ActiveScene()->m_updateComponents.end())
			{
				Scene::ActiveScene()->m_updateComponents.erase(sceneUpdateComponent);
			}
		}

		::Delete(*it);
		m_components.erase(it);
	}
}

void Entity::RemoveAllComponenets()
{
	IS_PROFILE_FUNCTION();

	for (auto it = m_components.begin(); it != m_components.end(); ++it)
	{
		(*it)->OnDestroy();
		::Delete(*it);
	}

	m_components.clear();
	m_componetBitset.clear();
}
