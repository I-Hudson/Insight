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

private:
	template<typename T>
	T& AddComponent(const EntityID& entity);
	template<typename T>
	T& GetComponent(const EntityID& entity);

	template<typename T>
	bool HasComponent(const EntityID& entity);

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
inline T& EntityManager::AddComponent(const EntityID& entity)
{
	m_entityData.at(entity).Signature.set(m_componentManager.GetComponentType<T>());
	return m_componentManager.AddComponent(entity, T(this, entity));
}

template<typename T>
inline T& EntityManager::GetComponent(const EntityID& entity)
{
	return m_componentManager.GetComponent<T>(entity);
}

template<typename T>
bool EntityManager::HasComponent(const EntityID& entity)
{
	return m_entityData.at(entity).Signature.test(m_componentManager.GetComponentType<T>());
}