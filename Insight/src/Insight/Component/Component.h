#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Core/Object.h"
#include "Insight/Serialization/Serializable.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/RTTI/RTTI.h"

class Entity;
class Scene;


class IS_API Component : 
	public Object
	, public Serialization::Serializable
{
public:
	Component(SharedPtr<Entity> owner)
		: Object()
		, Serialization::Serializable(this, true)
		, m_owner(owner)
		, m_isDirty(true)
		, m_updateEveryFarme(true)
		, m_componentId(-1)
	{
		IS_PROPERTY(bool, m_allowRemovale, "Allow_Removable", 0);
	}
	virtual ~Component() {}

	void SetEntity(SharedPtr<Entity> entity) { m_owner = entity; }
	WeakPtr<Entity> GetEntity() const { return m_owner; }

	template<typename T>
	SharedPtr<T> GetComponent();

	const bool& IsDirty() const { return m_isDirty; }

	virtual void OnCreate() { }
	virtual void OnUpdate(const float& deltaTime) { }
	virtual void OnDestroy() { }

protected:
	bool m_isDirty;
	bool m_updateEveryFarme;
	size_t m_componentId;
	bool m_allowRemovale;

private:
	void Init() {  }
	void Clean() { m_isDirty = false; }

private:
	WeakPtr<Entity> m_owner;
	std::string m_uuid;

	friend Entity;
	friend Scene;
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