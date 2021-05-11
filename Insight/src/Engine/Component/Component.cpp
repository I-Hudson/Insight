#include "ispch.h"
#include "Engine/Component/Component.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityManager.h"
#include "Engine/Scene/Scene.h"

Component::Component()
	: m_entityManager(nullptr), m_entityID(-1)
{ }

Component::Component(EntityManager* entityManager, EntityID entity)
	: m_entityManager(entityManager), m_entityID(entity)
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
