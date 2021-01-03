#include "ispch.h"
#include "Entity.h"

#include "Insight/Scene/Scene.h"
#include "Insight/Component/Component.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Model/Model.h"
#include "Insight/Instrumentor/Instrumentor.h"

#include "Insight/Core/Log.h"

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

void Entity::Serialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force)
{
	if (GetParent().expired() || force)
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

void Entity::Deserialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force)
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
			if (SharedPtr<Component> componentObject = CreateInstanceFromType<Component>(ptr->GetValue()))
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
			if (SharedPtr<Entity> childObject = CreateInstanceFromType<Entity>(ptr->GetValue()))
			{
				childObject->Deserialize(child);
				m_children.push_back(childObject);
			}
			IS_CORE_ERROR("[Entity::Deserialize] Child entity is null.");
		}
	}
}

bool Entity::HasComponent(const Insight::Type& type)
{
	return m_componetBitset[GetComponentID(type)];
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

void Entity::RemoveComponent(const std::string& uuid)
{
	auto it = std::find_if(m_components.begin(), m_components.end(), [uuid](SharedPtr<Component> ptr)
		{
			return ptr->GetUUID() == uuid;
		});

	if (it != m_components.end())
	{
		(*it)->OnDestroy();

		m_componetBitset[GetComponentID((*it)->GetType())] = false;

		if (m_attachedToScene)
		{
			auto sceneUpdateComponent = std::find_if(Insight::Scene::ActiveScene()->m_updateComponents.begin(),
				Insight::Scene::ActiveScene()->m_updateComponents.end(), [it](SharedPtr<Component> ptr)
				{
					return (*it)->GetUUID() == ptr->GetUUID();
				});
			if (sceneUpdateComponent != Insight::Scene::ActiveScene()->m_updateComponents.end())
			{
				Insight::Scene::ActiveScene()->m_updateComponents.erase(sceneUpdateComponent);
			}
		}

		(*it).reset();
		m_components.erase(it);
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
	m_componetBitset.clear();
}
