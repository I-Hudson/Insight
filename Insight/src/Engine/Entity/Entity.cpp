#include "ispch.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityManager.h"

REGISTER_DEF_TYPE(Entity);

Entity::Entity()
	: Serializable(this, false), m_entityManager(nullptr), m_entityID(-1), m_parentEntityID(-1)
{ }

Entity::Entity(EntityManager* entityManager, EntityID id)
	: Serializable(this, false), m_entityManager(entityManager), m_entityID(id), m_parentEntityID(-1)
{ }

Entity::~Entity()
{ }

void Entity::SetName(const std::string& name)
{
	if (!IsValid())
	{
		return;
	}
	EntityData& data = m_entityManager->GetEntityData(m_entityID);
	data.Name = name;
}

void Entity::Serialize(Serialization::SerializableElement* element, bool force)
{ }

void Entity::Deserialize(Serialization::SerializableElement* element, bool force)
{ }

void Entity::SetParent(const EntityID& entity)
{
	if (!IsValid())
	{
		return;
	}
	m_parentEntityID = entity;
}

Entity Entity::GetParent()
{
	if (!IsValid())
	{
		return Entity(nullptr, -1);
	}
	return m_entityManager->GetEntity(m_parentEntityID); 
}
