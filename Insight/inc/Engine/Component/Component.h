#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Serialization/Serializable.h"

#include "Reflect.h"

class Entity;
class EntityManager;
class ComponentManager;

struct IS_API ComponentData
{ };

class Component : REFLECT_BASE(), public Object
{
public:
	Component();
	Component(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity);
	virtual ~Component();

	virtual void OnUpdate(const float& a_deltaTime) { }
	bool IsValid() { return m_entityID != -1; }
	Entity GetEntity();

	ComponentID GetComponentID() const { return m_componentID; }

	template<typename T>
	T& GetComponentData()
	{
		return m_componentManager->GetComponentData<T>(m_componentType, m_componentID);
	}

protected:
	ComponentID m_componentID;
	ComponentType m_componentType;
	ComponentManager* m_componentManager;
	EntityID m_entityID;
	EntityManager* m_entityManager;

	friend ComponentManager;
};
