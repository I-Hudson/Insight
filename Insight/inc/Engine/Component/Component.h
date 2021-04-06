#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Serialization/Serializable.h"
#include "Engine/Entitiy/Entity.h"
#include "Engine/RTTI/RTTI.h"

class Entity;
class Scene;


class IS_API Component : 
	public Object
	, public Serialization::Serializable
{
public:
	Component(Entity* owner)
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

	void SetEntity(Entity* entity) { m_owner = entity; }
	Entity* GetEntity() const { return m_owner; }

	template<typename T>
	T* GetComponent();

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
	Entity* m_owner;
	std::string m_uuid;

	friend Entity;
	friend Scene;
};

template<typename T>
inline T* Component::GetComponent()
{
	const bool result = std::is_base_of<Component, T>::value;
	IS_CORE_ASSERT(result, "'T' is not drevided from 'Component'");

	if (m_owner)
	{
		return m_owner->GetComponent<T>();
	}
	return {};
}