#pragma once

#include "Insight/Core.h"
#include "Insight/UUID.h"
#include "Insight/Serialization/Serializable.h"
#include "Insight/Entitiy/Entity.h"

class Entity;

namespace Insight
{
	class Scene;
}

class IS_API Component : 
	public Insight::UUID
	, public Insight::Serialization::Serializable
{
public:
	Component()
		: Insight::UUID()
		, Insight::Serialization::Serializable(this, true)
		, m_isDirty(true)
		, m_updateEveryFarme(true)
		, m_componentId(-1)
	{ }
	Component(Entity* owner)
		: Insight::UUID(), m_owner(owner)
		, Insight::Serialization::Serializable(this, true)
		, m_isDirty(true)
		, m_updateEveryFarme(true)
		, m_componentId(-1)
	{ }
	virtual ~Component() {}

	void SetEntity(Entity* entity) { m_owner = entity; }
	Entity* GetEntity() const { return m_owner; }

	template<typename T>
	T* GetComponent();

	const bool& IsDirty() const { return m_isDirty; }

	virtual void OnCreate() { IS_CORE_ASSERT(m_componentId != -1, "A component has not been given an id. 'm_componentId' must be set with 'GetComponentID<T>'"); }
	virtual void OnUpdate(const float& deltaTime) { }
	virtual void OnDestroy() { }

protected:
	bool m_isDirty;
	bool m_updateEveryFarme;
	size_t m_componentId;

private:
	void Clean() { m_isDirty = false; }

private:
	Entity* m_owner;
	std::string m_uuid;

	friend Entity;
	friend Insight::Scene;
};

template<typename T>
inline T* Component::GetComponent()
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	return m_owner->GetComponent<T>();
}