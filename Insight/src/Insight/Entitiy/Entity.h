#pragma once

#include "Insight/Core.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Serialization/Serializable.h"
#include "Insight/UUID.h"

#include <string>
#include <vector>
#include <bitset>
#include <type_traits>

class Component;
class Entity;
const size_t MaxComponents = 32;

struct EntityData
{
	std::string Name;
	Entity* Parent = nullptr;
	std::vector<Entity*> Children;
	std::vector<Component*> Components;
	std::bitset<MaxComponents> ComponetBitset;
	bool IsActive;
	bool ShowDebugInfo;
};

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

class IS_API Entity : public Insight::UUID
					, public Insight::Serialization::Serializable
{
public:
	Entity();
	Entity(const std::string& id);
	virtual ~Entity() override;

	static Entity* Create(const std::string& id = "");
	static Entity* CreateFromModel(Model* model);
	void Delete();

	void OnUpdate(const float deltaTime);

	void SetID(const std::string& id);
	std::string& GetID();

	bool& IsActive() { return m_data.IsActive; }
	bool& ShowDebugInfo() { return m_data.ShowDebugInfo; }

	void SetParent(Entity* parent) { m_data.Parent = parent; }
	const Entity* GetParent() const { return m_data.Parent; }

	Entity* AddChild(const std::string& childId = "");
	void AddChild(Entity* child);
	Entity* GetChild(int childIndex);
	unsigned int GetChildCount() { return static_cast<unsigned int>(m_data.Children.size()); }
	void RemoveChild(Entity* child);

	const std::vector<Component*>* GetAllComponents()
	{
		return &m_data.Components;
	}
	template<typename T>
	bool HasComponent();
	template <typename T>
	T* AddComponent();

	template <typename T>
	void RemoveComponent();
	template <typename T>
	void RemoveComponent(const std::string& uuid);

	template<typename T>
	T* GetComponent() const;
	template<typename T>
	T* GetComponent(const std::string& uuid) const;

	virtual void Serialize(tinyxml2::XMLNode* out, tinyxml2::XMLDocument* doc, bool force = false) override;
	virtual void Deserialize(tinyxml2::XMLNode* in, bool force = false) override;

private:

	void AddComponent(Component* component);
	void RemoveAllComponenets();

	EntityData m_data;

	friend Insight::Scene;

	REGISTER_DEC_TYPE(Entity);
};

template<typename T>
inline bool Entity::HasComponent()
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	return m_data.ComponetBitset[GetComponentID<T>()];
}

template<typename T>
inline T* Entity::AddComponent()
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	if (m_data.Components.size() < MaxComponents && !HasComponent<T>())
	{
		T* c = NEW_ON_HEAP(T, this);
		static_cast<Component*>(c)->OnCreate();
		
		m_data.Components.push_back(c);
		m_data.ComponetBitset[GetComponentID<T>()] = true;

		if (static_cast<Component*>(c)->m_updateEveryFarme)
		{
			Insight::Scene::ActiveScene()->m_updateComponents.push_back(c);
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
		for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
		{
			T* tempPtr = dynamic_cast<T*>(*it);
			if (tempPtr != nullptr)
			{
				(*it)->OnDestroy();
				DELETE_ON_HEAP((*it));

				m_data.ComponetBitset[GetComponentID<T>()] = false;
				m_data.Components.erase(it);

				Insight::Scene::ActiveScene()->m_updateComponents.erase(std::find(Insight::Scene::ActiveScene()->m_updateComponents.begin(),
					Insight::Scene::ActiveScene()->m_updateComponents.end(), it));

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
		for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
		{
			if ((*it)->GetUUID() == uuid)
			{
				(*it)->OnDestroy();
				DELETE_ON_HEAP((*it));

				m_data.ComponetBitset[GetComponentID<T>()] = false;
				m_data.Components.erase(it);

				Insight::Scene::ActiveScene()->m_updateComponents.erase(std::find(Insight::Scene::ActiveScene()->m_updateComponents.begin(),
					Insight::Scene::ActiveScene()->m_updateComponents.end(), it));

				break;
			}
		}
	}
}

template<typename T>
inline T* Entity::GetComponent() const
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
	{
		T* tempT = dynamic_cast<T*>(*it);
		if (tempT != nullptr)
		{
			return tempT;
		}
	}
	return nullptr;
}

template<typename T>
inline T* Entity::GetComponent(const std::string& uuid) const
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
	{
		if ((*it)->GetUUID() == uuid)
		{
			return *it;
		}
	}
	return nullptr;
}