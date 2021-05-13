#pragma once

#include "Engine/Entity/Entity.h"
#include "Engine/Entity/ComponentManager.h"
#include <queue>
#include <set>

class Scene;

class EntityManager
{
public:
	explicit EntityManager(Scene* scene, ComponentManager& componentManager);
	~EntityManager();

	Entity CreateEntity();
	void DestroyEntity(const EntityID& entity);

	Entity GetEntity(const EntityID entity);
	EntityData& GetEntityData(const EntityID& entity);

	std::vector<Entity> GetAliveEntities();

private:
	template<typename T>
	T& AddComponent(const EntityID& entity);

	template<typename T>
	T& GetComponent(const EntityID& entity);

	template<typename T>
	void RemoveComponent(const EntityID& entity);
	void RemoveComponent(const EntityID& entity, const Component& component);

	template<typename T>
	bool HasComponent(const EntityID& entity);

	Component& GetComponent(const EntityID& entity, const u32& componentIndex);

	std::vector<Component> GetAllComponent(const EntityID& entity);

private:
	std::queue<EntityID> m_entities;
	std::set<EntityID> m_aliveEntitiesIDs;
	std::array<EntityData, MAX_ENTITIES_COUNT> m_entityData;
	u32 m_aliveEntitites;

	Scene* m_scene;
	ComponentManager& m_componentManager;

	friend Entity;
};

template<typename T>
INLINE T& EntityManager::AddComponent(const EntityID& entity)
{
	m_entityData.at(entity).Signature.set(m_componentManager.GetComponentType<T>());
	T& component = m_componentManager.AddComponent<T>(entity, this);
	m_entityData.at(entity).ComponentIDs.push_back({ m_componentManager.GetComponentType<T>(), component.GetComponentID() });
	return component;
}

template<typename T>
INLINE T& EntityManager::GetComponent(const EntityID& entity)
{
	return m_componentManager.GetComponent<T>(entity);
}

template<typename T>
INLINE void EntityManager::RemoveComponent(const EntityID& entity)
{
	m_entityData.at(entity).Signature.reset(m_componentManager.GetComponentType<T>());
	auto itr = std::find_if(m_entityData.at(entity).ComponentIDs.begin(), m_entityData.at(entity).ComponentIDs.end(), [this, &entity](const std::pair<ComponentType, ComponentID>& pair)
	{
		return m_componentManager.GetComponentType<T>() == pair.first && m_componentManager.GetComponent<T>(entity).GetComponentID() == pair.second;
	});
	//ASSERT(itr != m_entityData.at(entity).ComponentIDs.end() && "[EntityManager::RemoveComponent] Component not listed on Entity.");
	m_entityData.at(entity).ComponentIDs.erase(itr);
	m_componentManager.RemoveComponent<T>(entity);
}

template<typename T>
INLINE bool EntityManager::HasComponent(const EntityID& entity)
{
	return m_entityData.at(entity).Signature.test(m_componentManager.GetComponentType<T>());
}