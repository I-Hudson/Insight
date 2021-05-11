#include "ispch.h"
#include "Engine/Entity/EntityManager.h"
#include "Engine/Scene/Scene.h"

EntityManager::EntityManager(Scene* scene)
	: m_scene(scene)
{
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

template<typename T, typename... Args>
inline T& EntityManager::AddComponent(const EntityID& entity, Args&&... args)
{
	ComponentManager& componentManager = m_scene->GetComponentManager();
	m_entityData.at(entity).Signature.set(componentManager.GetComponentType<T>());
	return componentManager.AddComponent(entity, T(std::forward<Args...>(std::move(args)...)));
}

template<typename T>
inline T& EntityManager::GetComponent(const EntityID& entity)
{
	ComponentManager& componentManager = m_scene->GetComponentManager();
	return componentManager.GetComponent<T>(entity);
}

template<typename T>
bool EntityManager::HasComponent(const EntityID& entity)
{
	ComponentManager& componentManager = m_scene->GetComponentManager();
	return m_entityData.at(entity).Signature.test(componentManager.GetComponentType<T>());
}
