#pragma once

#include "Engine/Entity/Entity.h"
#include <queue>
#include <set>

class Scene;

class EntityManager
{
public:
	explicit EntityManager(Scene* scene);
	~EntityManager();

	Entity CreateEntity();
	void DestroyEntity(const EntityID& entity);

	Entity GetEntity(const EntityID entity);
	EntityData& GetEntityData(const EntityID& entity);

private:
	template<typename T, typename... Args>
	T& AddComponent(const EntityID& entity, Args&&... args);
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

	friend Entity;
};