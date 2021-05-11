#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Serialization/Serializable.h"

class Entity;
class EntityManager;
class ComponentManager;

class IS_API Component
{
public:
	Component();
	Component(EntityManager* entityManager, EntityID entity);
	virtual ~Component();
	virtual void OnUpdate(const float& a_deltaTime) = 0;
	bool IsValid() { return m_entityID != -1; }
	Entity GetEntity();

protected:
	EntityID m_entityID;
	EntityManager* m_entityManager;

	friend ComponentManager;
};
