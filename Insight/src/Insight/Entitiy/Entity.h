#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Serialization/Serializable.h"
#include "Insight/Scene/Scene.h"
#include "Insight/Core/Object.h"

#include <string>
#include <vector>
#include <bitset>
#include <type_traits>

class Component;
class Entity;
const size_t MaxComponents = 32;

using ComponentID = size_t;
inline ComponentID GetComponentID()
{
	static ComponentID lastID = 0;
	return lastID++;
}

template<typename T>
inline ComponentID GetComponentID() noexcept
{
	static ComponentID typeID = GetComponentID();
	return typeID;
}

namespace Insight
{
	class Scene;
}

class Model;

class IS_API Entity : public Insight::Object
					, public Insight::Serialization::Serializable
					, public std::enable_shared_from_this<Entity>
{
public:
	Entity();
	Entity(const std::string& id);
	Entity(const std::string& id, bool attachToScene);
	virtual ~Entity() override;

	static SharedPtr<Entity> Create(const std::string& id = "");
	static SharedPtr<Entity> CreateFromModel(SharedPtr<Model> model);
	void Delete();

	void OnUpdate(const float deltaTime);

	void SetID(const std::string& id);
	std::string& GetID();

	bool& IsActive() { return m_isActive; }
	bool& ShowDebugInfo() { return m_showDebugInfo; }

	void SetParent(SharedPtr<Entity> parent) { m_parent = parent; }
	const WeakPtr<Entity> GetParent() const { return m_parent; }

	SharedPtr<Entity> AddChild(const std::string& childId = "");
	void AddChild(SharedPtr<Entity> child);
	SharedPtr<Entity> GetChild(int childIndex);
	unsigned int GetChildCount() { return static_cast<unsigned int>(m_children.size()); }
	void RemoveChild(SharedPtr<Entity> child);

	const std::vector<SharedPtr<Component>>& GetAllComponents() const {return m_components; }
	template<typename T>
	bool HasComponent();
	template <typename T>
	SharedPtr<T> AddComponent();

	template <typename T>
	void RemoveComponent();
	template <typename T>
	void RemoveComponent(const std::string& uuid);

	template<typename T>
	SharedPtr<T> GetComponent() const;
	template<typename T>
	SharedPtr<T> GetComponent(const std::string& uuid) const;

	virtual void Serialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force = false) override;
	virtual void Deserialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force = false) override;

private:

	void AddComponent(SharedPtr<Component> component);
	void RemoveAllComponenets();

	std::string m_name;
	WeakPtr<Entity> m_parent;
	std::vector<SharedPtr<Entity>> m_children;
	std::vector<SharedPtr<Component>> m_components;
	std::bitset<MaxComponents> m_componetBitset;
	bool m_isActive;
	bool m_showDebugInfo;
	bool m_attachedToScene = true;

	friend Insight::Scene;

	REGISTER_DEC_TYPE(Entity);
};

template<typename T>
inline bool Entity::HasComponent()
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	return m_componetBitset[GetComponentID<T>()];
}

template<typename T>
inline SharedPtr<T> Entity::AddComponent()
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	if (m_components.size() < MaxComponents && !HasComponent<T>())
	{
		SharedPtr<T> c = Object::CreateObject<T>(this->shared_from_this());
		StaticPointerCast<Component>(c)->OnCreate();
		
		m_components.push_back(c);
		m_componetBitset[GetComponentID<T>()] = true;

		if (StaticPointerCast<Component>(c)->m_updateEveryFarme)
		{
			if (m_attachedToScene)
			{
				Insight::Scene::ActiveScene()->m_updateComponents.push_back(c);
			}
		}

		return c;
	}
	return nullptr;
}

template<typename T>
inline void Entity::RemoveComponent()
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	if (HasComponent<T>())
	{
		for (auto it = m_components.begin(); it != m_components.end(); ++it)
		{
			SharedPtr<T> tempPtr = DynamicPointerCast<T>(*it);
			if (tempPtr != nullptr)
			{
				(*it)->OnDestroy();
				(*it).reset();

				m_componetBitset[GetComponentID<T>()] = false;
				m_components.erase(it);

				if (m_attachedToScene)
				{
					Insight::Scene::ActiveScene()->m_updateComponents.erase(std::find(Insight::Scene::ActiveScene()->m_updateComponents.begin(),
																					  Insight::Scene::ActiveScene()->m_updateComponents.end(), it));
				}

				break;
			}
		}
	}
}

template<typename T>
inline void Entity::RemoveComponent(const std::string& uuid)
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	if (HasComponent<T>())
	{
		for (auto it = m_components.begin(); it != m_components.end(); ++it)
		{
			if ((*it)->GetUUID() == uuid)
			{
				(*it)->OnDestroy();
				(*it).reset();

				m_componetBitset[GetComponentID<T>()] = false;
				m_components.erase(it);

				if (m_attachedToScene)
				{
					Insight::Scene::ActiveScene()->m_updateComponents.erase(std::find(Insight::Scene::ActiveScene()->m_updateComponents.begin(),
																					  Insight::Scene::ActiveScene()->m_updateComponents.end(), it));
				}
				break;
			}
		}
	}
}

template<typename T>
inline SharedPtr<T> Entity::GetComponent() const
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	for (auto it = m_components.begin(); it != m_components.end(); ++it)
	{
		SharedPtr<T> tempT = DynamicPointerCast<T>(*it);//  dynamic_cast<SharedPtr<T>>((*it).get());
		if (tempT != nullptr)
		{
			return tempT;
		}
	}
	return nullptr;
}

template<typename T>
inline SharedPtr<T> Entity::GetComponent(const std::string& uuid) const
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	for (auto it = m_components.begin(); it != m_components.end(); ++it)
	{
		if ((*it)->GetUUID() == uuid)
		{
			return *it;
		}
	}
	return nullptr;
}