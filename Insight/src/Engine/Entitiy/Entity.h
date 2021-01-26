#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Serialization/Serializable.h"
#include "Engine/Scene/Scene.h"

#include <string>
#include <vector>
#include <bitset>
#include <type_traits>

class Component;
class Entity;
const size_t MaxComponents = 32;

inline U64 GetComponentID(const Type& type) noexcept
{
	return type.GetTypeHash();
}

class Scene;
class Model;

class IS_API Entity : public Object
					, public Serialization::Serializable
{
public:
	Entity();
	Entity(const std::string& id);
	Entity(const std::string& id, bool attachToScene);
	virtual ~Entity() override;

	static Entity* New(const std::string& id = "Untitled");
	static Entity* CreateFromModel(Model* model);
	void Delete();

	void OnUpdate(const float deltaTime);

	void SetID(const std::string& id);
	std::string& GetID();

	bool& IsActive() { return m_isActive; }
	bool& ShowDebugInfo() { return m_showDebugInfo; }

	void SetParent(Entity* parent) { m_parent = parent; }
	const Entity* GetParent() const { return m_parent; }

	Entity* AddChild(const std::string& childId = "");
	void AddChild(Entity* child);
	Entity* GetChild(int childIndex);
	unsigned int GetChildCount() { return static_cast<unsigned int>(m_children.size()); }
	void RemoveChild(Entity* child);

	const std::vector<Component*>& GetAllComponents() const {return m_components; }
	template<typename T>
	bool HasComponent();
	template <typename T>
	T* AddComponent();

	template <typename T>
	void RemoveComponent();
	void RemoveComponent(const std::string& uuid);

	template<typename T>
	T* GetComponent() const;
	template<typename T>
	T* GetComponent(const std::string& uuid) const;

	virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
	virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

private:

	bool HasComponent(const Type& type);
	void AddComponent(Component* component);
	void RemoveAllComponenets();

	std::string m_name;
	Entity* m_parent;
	std::vector<Entity*> m_children;
	std::vector<Component*> m_components;
	std::unordered_map<U64, U8> m_componetBitset;
	bool m_isActive;
	bool m_showDebugInfo;
	bool m_attachedToScene = true;

	friend Scene;

	REGISTER_DEC_TYPE(Entity);
};

template<typename T>
inline bool Entity::HasComponent()
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	Type type;
	type.SetType<T>();
	return HasComponent(type);
}

template<typename T>
inline T* Entity::AddComponent()
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	if (m_components.size() < MaxComponents && !HasComponent<T>())
	{
		T* c = ::New<T>(this);
		Component* base = static_cast<Component*>(c);
		base->OnCreate();
		
		m_components.push_back(c);
		m_componetBitset[GetComponentID(base->GetType())] = true;

		if (base->m_updateEveryFarme)
		{
			if (m_attachedToScene)
			{
				Scene::ActiveScene()->m_updateComponents.push_back(c);
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
			T* tempPtr = dynamic_cast<T*>(*it);
			if (tempPtr != nullptr)
			{
				Component* base = static_cast<Component*>(c);

				(*it)->OnDestroy();
				::Delete(*it);

				m_componetBitset[GetComponentID(base->GetType())] = false;
				m_components.erase(it);

				if (m_attachedToScene)
				{
					Scene::ActiveScene()->m_updateComponents.erase(std::find(Scene::ActiveScene()->m_updateComponents.begin(),
																					  Scene::ActiveScene()->m_updateComponents.end(), it));
				}

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

	for (auto it = m_components.begin(); it != m_components.end(); ++it)
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

	for (auto it = m_components.begin(); it != m_components.end(); ++it)
	{
		if ((*it)->GetUUID() == uuid)
		{
			return *it;
		}
	}
	return nullptr;
}