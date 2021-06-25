
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

std::vector<Entity> EntityManager::GetAliveEntities()
{
	std::vector<Entity> aliveEntities;
	for (auto& e : m_aliveEntitiesIDs)
	{
		aliveEntities.push_back(Entity(this, e));
	}
	return aliveEntities;
}

void EntityManager::RemoveComponent(const EntityID& entity, const Component& component)
{
	m_entityData.at(entity).Signature.reset(component.GetComponentType());
	auto itr = std::find_if(m_entityData.at(entity).ComponentIDs.begin(), m_entityData.at(entity).ComponentIDs.end(), [this, &entity, &component](const std::pair<ComponentType, ComponentID>& pair)
	{
		return component.GetComponentType() == pair.first && component.GetComponentID() == pair.second;
	});
	ASSERT(itr != m_entityData.at(entity).ComponentIDs.end() && "[EntityManager::RemoveComponent] Component not listed on Entity.");
	m_entityData.at(entity).ComponentIDs.erase(itr);
	m_componentManager.RemoveComponent(entity, component.GetComponentType(), component.GetComponentID());
}

Component& EntityManager::GetComponent(const EntityID& entity, const u32& componentIndex)
{
	return m_componentManager.GetComponentBaseRef(entity, GetEntityData(entity).ComponentIDs.at(componentIndex).first);
}

std::vector<Component> EntityManager::GetAllComponent(const EntityID& entity)
{
	return m_componentManager.GetAllComponents(entity);
}
