#include "ispch.h"
#include "Engine/Entity/EntityManager.h"

EntityManager::EntityManager(Scene* scene, ComponentManager& componentManager)
	: m_scene(scene), m_componentManager(componentManager), m_aliveEntitites(0)
{
	for (u32 i = 0; i < MAX_ENTITIES_COUNT; ++i)
	{
		m_entities.push(i);
	}
}

EntityManager::~EntityManager()
{
}

Entity EntityManager::CreateEntity()
{
	Entity e(this, m_entities.front());
	m_aliveEntitiesIDs.insert(m_entities.front());
	m_entities.pop();
	++m_aliveEntitites;
	return e;
}

void EntityManager::DestroyEntity(const EntityID& entity)
{
	ASSERT(entity < MAX_ENTITIES_COUNT && "[EntityManager::DestroyEntity] Entity out of range.");

	m_entities.push(entity);
	m_aliveEntitiesIDs.erase(entity);
	--m_aliveEntitites;
}

Entity EntityManager::GetEntity(const EntityID entity)
{
	if (m_aliveEntitiesIDs.find(entity) != m_aliveEntitiesIDs.end())
	{
		return Entity(this, entity);
	}
	return Entity(nullptr, -1);
}

EntityData& EntityManager::GetEntityData(const EntityID& entity)
{
	return m_entityData.at(entity);
}
