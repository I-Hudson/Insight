#pragma once

#include "Insight/Core.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Serialization/Serializable.h"
#include "Insight/UUID.h"

#include <string>
#include <vector>
#include <bitset>

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

	void SetID(const std::string& id);
	const std::string& GetID() const;

	bool IsActive() const { return m_data.IsActive; }

	void SetParent(Entity* parent) { m_data.Parent = parent; }
	const Entity* GetParent() const { return m_data.Parent; }

	Entity* AddChild(const std::string& childId = "");
	void AddChild(Entity* child);
	Entity* GetChild(int childIndex);
	unsigned int GetChildCount() { return m_data.Children.size(); }
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

	virtual void Serialize(std::ostream& out) override;
	virtual void Deserialize(std::istream& in) override;

private:
	void RemoveAllComponenets();

	EntityData m_data;

	REGISTER_DEC_TYPE(Entity);
};

template<typename T>
inline bool Entity::HasComponent()
{
	return m_data.ComponetBitset[GetComponentID<T>()];
}

template<typename T>
inline T* Entity::AddComponent() 
{
	if (m_data.Components.size() < MaxComponents && !HasComponent<T>())
	{
		T* c = NEW_ON_HEAP(T, this);
		m_data.Components.push_back(c);
		m_data.ComponetBitset[GetComponentID<T>()] = true;

		return c;
	}
	return nullptr;
}

template<typename T>
inline void Entity::RemoveComponent()
{
	if (HasComponent<T>())
	{
		for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
		{
			T* tempPtr = dynamic_cast<T*>(*it);
			if (tempPtr != nullptr)
			{
				DELETE_ON_HEAP((*it));
				m_data.ComponetBitset[GetComponentID<T>()] = false;
				m_data.Components.erase(it);
				break;
			}
		}
	}
}

template<typename T>
inline void Entity::RemoveComponent(const std::string& uuid)
{
	if (HasComponent<T>())
	{
		for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
		{
			if ((*it)->GetUUID() == uuid)
			{
				DELETE_ON_HEAP((*it));
				m_data.ComponetBitset[GetComponentID<T>()] = false;
				m_data.Components.erase(it);
				break;
			}
		}
	}
}

template<typename T>
inline T* Entity::GetComponent() const
{
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
	for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
	{
		if ((*it)->GetUUID() == uuid)
		{
			return *it;
		}
	}
	return nullptr;
}