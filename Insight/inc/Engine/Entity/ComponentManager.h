#pragma once

#include "Engine/Core/Common.h"
#include <array>

class ComponentManager;

class IComponentArray
{
public:
	virtual ~IComponentArray() = default;
	virtual void Update(const float& deltaTime) = 0;
	virtual void EntityDestroyed(EntityID entity) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray
{
public:
	T& AddComponent(EntityID entity, const T& component)
	{
		ASSERT(m_entityToIndexMap.find(entity) == m_entityToIndexMap.end() &&
			   "[ComponentArray::AddComponent] Component added to same entity more than once.");
	
		u32 newIndex = m_size;
		m_entityToIndexMap[entity] = newIndex;
		m_indexToEntityMap[newIndex] = entity;
		m_componentArray[newIndex] = component;
		++m_size;
		return m_componentArray.at(newIndex);
	}

	void RemoveComponent(EntityID entity)
	{
		ASSERT(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end() && 
			   "[ComponentArray::RemoveComponent] Removing non-existent component.");

		// Copy element at end into deleted element's place to maintain density
		u32 indexOfRemovedEntity = m_entityToIndexMap[entity];
		u32 indexOfLastElement = m_size - 1;
		m_componentArray[indexOfRemovedEntity] = m_componentArray[indexOfLastElement];

		// Update map to point to moved spot
		EntityID entityOfLastElement = m_indexToEntityMap[indexOfLastElement];
		m_entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
		m_entityToIndexMap[indexOfRemovedEntity] = entityOfLastElement;

		m_entityToIndexMap.erase(entity);
		m_indexToEntityMap.erase(indexOfLastElement);

		--m_size;
	}

	T& GetComponent(EntityID entity)
	{
		ASSERT(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end() &&
			   "[ComponentArray::GetComponent] Retrieving non-existent component.");

		// Return a reference to the entity's component
		return m_componentArray[m_entityToIndexMap[entity]];
	}

	virtual void Update(const float& deltaTime) override
	{
		IS_PROFILE_FUNCTION();
		for (auto& component : m_componentArray)
		{
			if (component.IsValid())
			{
				component.OnUpdate(deltaTime);
			}
		}
	}

	virtual void EntityDestroyed(EntityID entity) override
	{
		if (m_entityToIndexMap.find(entity) != m_entityToIndexMap.end())
		{
			// Remove the entity's component if it existed
			RemoveComponent(entity);
		}
	}

private:
	// The packed array of components (of generic type T),
	// set to a specified maximum amount, matching the maximum number
	// of entities allowed to exist simultaneously, so that each entity
	// has a unique spot.
	std::array<T, MAX_ENTITIES_COUNT> m_componentArray;

	// Map from an entity ID to an array index.
	std::unordered_map<EntityID, u32> m_entityToIndexMap;

	// Map from an array index to an entity ID.
	std::unordered_map<u32, EntityID> m_indexToEntityMap;

	// Total size of valid entries in the array.
	u32 m_size;

	friend ComponentManager;
};

class ComponentManager
{
public:
	ComponentManager();
	~ComponentManager();

	void Update(const float& deltaTime);

	template<typename T>
	void RegisterComponent()
	{
		const char* typeName = typeid(T).name();
		ASSERT(m_componentTypes.find(typeName) == m_componentTypes.end() &&
			   "[ComponentManager::RegisterComponent] Registering component type more than once.");

		m_componentTypes.insert({ typeName, m_nextComponentType });

		// Create a ComponentArray pointer and add it to the component arrays map
		m_componentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });

		// Increment the value so that the next component registered will be different
		++m_nextComponentType;
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		const char* typeName = typeid(T).name();

		ASSERT(m_componentTypes.find(typeName) != m_componentTypes.end() &&
			   "[ComponentManager::GetComponentType] Component not registered before use.");

		return m_componentTypes.at(typeName);
	}

	template<typename T>
	T& AddComponent(const EntityID& entity, const T& component)
	{
		// Add a component to the array for an entity
		return GetComponentArray<T>()->AddComponent(entity, component);
	}

	template<typename T>
	void RemoveComponent(const EntityID& entity)
	{
		// Remove a component from the array for an entity
		GetComponentArray<T>()->RemoveComponent(entity);
	}

	template<typename T>
	T& GetComponent(const EntityID& entity)
	{
		// Get a reference to a component from the array for an entity
		return GetComponentArray<T>()->GetComponent(entity);
	}

	template<typename T>
	std::array<T, MAX_ENTITIES_COUNT>& GetAllComponents()
	{
		return GetComponentArray<T>()->m_componentArray;
	}

	void EntityDestroyed(const EntityID& entity)
	{
		// Notify each component array that an entity has been destroyed
		// If it has a component for that entity, it will remove it
		for (auto const& pair : m_componentArrays)
		{
			auto const& component = pair.second;

			component->EntityDestroyed(entity);
		}
	}

private:
	// Map from type string pointer to a component type
	std::unordered_map<const char*, ComponentType> m_componentTypes{};

	// Map from type string pointer to a component array
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_componentArrays{};

	// The component type to be assigned to the next registered component - starting at 0
	ComponentType m_nextComponentType{};

	// Convenience function to get the statically casted pointer to the ComponentArray of type T.
	template<typename T>
	std::shared_ptr<ComponentArray<T>> GetComponentArray()
	{
		const char* typeName = typeid(T).name();

		assert(m_componentTypes.find(typeName) != m_componentTypes.end() && 
			   "[ComponentManager::GetComponentArray] Component not registered before use.");

		return std::static_pointer_cast<ComponentArray<T>>(m_componentArrays.at(typeName));
	}
};
