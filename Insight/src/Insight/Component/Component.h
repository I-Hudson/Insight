#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Core/Object.h"
#include "Insight/Serialization/Serializable.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/RTTI/RTTI.h"

class Entity;

namespace Insight
{
	class Scene;
}

class IS_API Component : 
	public Insight::Object
	, public Insight::Serialization::Serializable
{
public:
	Component()
		: Insight::Object()
		, Insight::Serialization::Serializable(this, true)
		, m_isDirty(true)
		, m_updateEveryFarme(true)
		, m_componentId(-1)
	{ }

	Component(SharedPtr<Entity> owner)
		: Insight::Object()
		, Insight::Serialization::Serializable(this, true)
		, m_owner(owner)
		, m_isDirty(true)
		, m_updateEveryFarme(true)
		, m_componentId(-1)
	{ }
	virtual ~Component() {}

	void SetEntity(SharedPtr<Entity> entity) { m_owner = entity; }
	WeakPtr<Entity> GetEntity() const { return m_owner; }

	template<typename T>
	SharedPtr<T> GetComponent();

	const bool& IsDirty() const { return m_isDirty; }

	virtual void OnCreate() { IS_CORE_ASSERT(m_componentId != -1, "A component has not been given an id. 'm_componentId' must be set with 'GetComponentID<T>'"); }
	virtual void OnUpdate(const float& deltaTime) { }
	virtual void OnDestroy() { }

	virtual std::string GetTypeName() const { return "Component"; }

protected:
	bool m_isDirty;
	bool m_updateEveryFarme;
	size_t m_componentId;

private:
	void Clean() { m_isDirty = false; }

private:
	WeakPtr<Entity> m_owner;
	std::string m_uuid;

	friend Entity;
	friend Insight::Scene;
};

template<typename T>
inline SharedPtr<T> Component::GetComponent()
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	if (SharedPtr<Entity> parnetPtr = m_owner.lock())
	{
		return parnetPtr->GetComponent<T>();
	}
	return {};
}