#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Component/Component.h"
#include <array>

class ComponentManager;
class EntityManager;

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
	T& AddComponent(EntityID entity, ComponentManager* componentManger, ComponentType componentType, EntityManager* entityManger)
	{
		ASSERT(m_entityToIndexMap.find(entity) == m_entityToIndexMap.end() &&
			   "[ComponentArray::AddComponent] Component added to same entity more than once.");
	
		u32 newIndex = m_size;
		m_entityToIndexMap[entity] = newIndex;
		m_indexToEntityMap[newIndex] = entity;
		m_componentArray[newIndex] = T(componentManger, newIndex, componentType, entityManger, entity);
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
		return m_componentArray.at(m_entityToIndexMap.at(entity));
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
			return;
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

class IComponentDataArray 
{
public:
	virtual ~IComponentDataArray() = default;
	virtual void AddComponentData(ComponentID component) = 0;
	virtual void RemoveComponentData(ComponentID component) = 0;
};

template<typename T>
class ComponentDataArray : public IComponentDataArray
{
public:
	virtual void AddComponentData(ComponentID component) override
	{
		ASSERT(m_componentToIndexMap.find(component) == m_componentToIndexMap.end() &&
			   "[ComponentDataArray::AddComponentData] ComponentData added to same component more than once.");

		u32 newIndex = m_size;
		m_componentToIndexMap[component] = newIndex;
		m_indexToComponentMap[newIndex] = component;
		m_componentDataArray[newIndex] = T();
		++m_size;
	}

	T& GetComponentData(ComponentID component)
	{
		ASSERT(m_componentToIndexMap.find(component) != m_componentToIndexMap.end() &&
			   "[ComponentDataArray::GetComponentData] Retrieving non-existent component data.");

		// Return a reference to the entity's component
		return m_componentDataArray.at(m_componentToIndexMap.at(component));
	}

	virtual void RemoveComponentData(ComponentID component) override
	{
		ASSERT(m_componentToIndexMap.find(component) != m_componentToIndexMap.end() &&
			   "[ComponentDataArray::RemoveComponentData] Removing non-existent component data.");

		// Copy element at end into deleted element's place to maintain density
		u32 indexOfRemovedEntity = m_componentToIndexMap[component];
		u32 indexOfLastElement = m_size - 1;
		m_componentDataArray[indexOfRemovedEntity] = m_componentDataArray[indexOfLastElement];

		// Update map to point to moved spot
		EntityID entityOfLastElement = m_indexToComponentMap[indexOfLastElement];
		m_componentToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
		m_componentToIndexMap[indexOfRemovedEntity] = entityOfLastElement;

		m_componentToIndexMap.erase(component);
		m_indexToComponentMap.erase(indexOfLastElement);

		--m_size;
	}

private:
	std::array<T, MAX_ENTITIES_COUNT> m_componentDataArray;
	// Track the component data via each component not entity. Thie will allow for more than on component on an entity.
	std::unordered_map<ComponentID, u32> m_componentToIndexMap;
	std::unordered_map<u32, ComponentID> m_indexToComponentMap;
	u32 m_size;

	friend ComponentManager;
};

class ComponentManager
{
public:
	ComponentManager();
	~ComponentManager();

	void Update(const float& deltaTime);

	template<typename T, typename TData = ComponentData>
	void RegisterComponent()
	{
		const char* typeName = typeid(T).name();
		ASSERT(m_componentTypes.find(typeName) == m_componentTypes.end() &&
			   "[ComponentManager::RegisterComponent] Registering component type more than once.");

		m_componentTypes.insert({ typeName, m_nextComponentType });
		ComponentType componentType = GetComponentType<T>();

		// Create a ComponentArray pointer and add it to the component arrays map
		m_componentArrays.insert({ componentType, ::New<ComponentArray<T>>() });

		// Create a ComponentDataArray pointer for the Component just registered.
		m_componentDataArrays.insert({ componentType, ::New<ComponentDataArray<TData>>() });

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
	T& AddComponent(const EntityID& entity, EntityManager* entityManager)
	{
		// Add a component to the array for an entity
		T& component = GetComponentArray<T>()->AddComponent(entity, this, GetComponentType<T>(), entityManager);
		m_componentDataArrays.at(GetComponentType<T>())->AddComponentData(component.GetComponentID());
		return component;
	}

	template<typename T>
	void RemoveComponent(const EntityID& entity)
	{
		// Remove a component from the array for an entity
		T& component = GetComponent<T>(entity);
		m_componentDataArrays.at(GetComponentType<T>())->RemoveComponentData(component.GetComponentID());
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

	std::vector<Component> GetAllComponents(const EntityID& entity);

	template<typename T>
	T& GetComponentData(ComponentType componentType, ComponentID componentID)
	{
		return GetComponentDataArray<T>(componentType)->GetComponentData(componentID);
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
	std::unordered_map<ComponentType, IComponentArray*> m_componentArrays{};

	// Map for all the component data for each component.
	std::unordered_map<ComponentType, IComponentDataArray*> m_componentDataArrays{};

	// The component type to be assigned to the next registered component - starting at 0
	ComponentType m_nextComponentType{};

	// Convenience function to get the statically casted pointer to the ComponentArray of type T.
	template<typename T>
	ComponentArray<T>* GetComponentArray()
	{
		const char* typeName = typeid(T).name();
		ASSERT(m_componentTypes.find(typeName) != m_componentTypes.end() && 
			   "[ComponentManager::GetComponentArray] Component not registered before use.");

		u32 componentType = GetComponentType<T>();
		return static_cast<ComponentArray<T>*>(m_componentArrays.at(componentType));
	}

	template<typename T>
	ComponentDataArray<T>* GetComponentDataArray(ComponentType componentType)
	{
		//ASSERT(m_componentTypes.find(typeName) != m_componentTypes.end() &&
		//	   "[ComponentManager::GetComponentDataArray] Component data not registered before use.");

		return static_cast<ComponentDataArray<T>*>(m_componentDataArrays.at(componentType));
	}
};
