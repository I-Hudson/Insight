#include "ispch.h"
#include "Engine/Component/Component.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityManager.h"
#include "Engine/Scene/Scene.h"

Component::Component()
	: m_componentManager(nullptr)
	, m_componentID(-1)
	, m_componentType(-1)
	, m_entityManager(nullptr)
	, m_entityID(-1)
{ }

Component::Component(ComponentManager* componentManger, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity)
	: m_componentManager(componentManger)
	, m_componentID(componentID)
	, m_componentType(componentType)
	, m_entityManager(entityManager)
	, m_entityID(entity)
{ }

Component::~Component()
{ }

Entity Component::GetEntity()
{
	if (m_entityManager)
	{
		return m_entityManager->GetEntity(m_entityID);
	}
	return Entity(nullptr, -1);
}

ComponentData& Component::GetComponentData()
{
	return m_componentManager->GetComponentDataBaseRef(m_componentType, m_componentID);
}
