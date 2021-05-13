#include "ispch.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityManager.h"

REGISTER_DEF_TYPE(Entity);

Entity::Entity()
	: Serializable(this, false), m_entityManager(nullptr), m_entityID(-1)
{ }

Entity::Entity(EntityManager* entityManager, EntityID id)
	: Serializable(this, false), m_entityManager(entityManager), m_entityID(id)
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

std::string Entity::GetName()
{
	if (!IsValid())
	{
		return "";
	}
	return m_entityManager->GetEntityData(m_entityID).Name;
}

u32 Entity::GetChildCount()
{
	return static_cast<u32>(GetEntiyData().EntityChildrenIDs.size());
}

Entity Entity::GetChild(const EntityID& entity)
{
	ASSERT(entity < static_cast<u32>(GetEntiyData().EntityChildrenIDs.size())  && "[Entity::GetChild] Entity is not a child.");
	if (!IsValid())
	{
		return Entity(nullptr, -1);
	}
	return m_entityManager->GetEntity(GetEntiyData().EntityChildrenIDs.at(entity));
}

void Entity::RemoveComponent(const Component& component)
{
	m_entityManager->RemoveComponent(m_entityID, component);
}

//std::vector<Component> Entity::GetAllComponents()
//{
//	if (!IsValid())
//	{
//		return {};
//	}
//	return m_entityManager->GetAllComponent(m_entityID);
//}

u32 Entity::GetComponentCount()
{
	return static_cast<u32>(GetEntiyData().Signature.count());
}

Component& Entity::GetComponent(const u32& index)
{
	return m_entityManager->GetComponent(m_entityID, index);
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
	if (entity == -1)
	{
		Entity parent = GetParent();
		if (parent.IsValid())
		{
			auto itr = std::find(GetEntiyData().EntityChildrenIDs.begin(), GetEntiyData().EntityChildrenIDs.end(), entity);
			GetParent().GetEntiyData().EntityChildrenIDs.erase(itr);
		}
	}
	else
	{
		GetEntiyData().ParentEntityID = entity;
		Entity parent = GetParent();
		if (parent.IsValid())
		{
			GetParent().GetEntiyData().EntityChildrenIDs.push_back(m_entityID);
		}
	}

}

Entity Entity::GetParent()
{
	if (!IsValid())
	{
		return Entity(nullptr, -1);
	}
	return m_entityManager->GetEntity(GetEntiyData().ParentEntityID);
}

EntityData& Entity::GetEntiyData()
{
	return m_entityManager->GetEntityData(m_entityID);
}