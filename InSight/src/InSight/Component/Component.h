#pragma once

#include "Insight/Core.h"
#include "Insight/UUID.h"
#include "Insight/Serialization/Serializable.h"

class Entity;

namespace Insight
{
	class Scene;
}

enum class ComponentType
{
	AMBIENT_LIGHT,
	CAMERA,
	DIRECTIONAL_LIGHT,
	MESH,
	POINT_LIGHT,
	SPOT_LIGHT,
	TRANSFORM
};

class IS_API Component : 
	public Insight::UUID
	, public Insight::Serialization::Serializable
{
public:
	Component()
		: Insight::UUID()
		, Insight::Serialization::Serializable(this, true)
		, m_isDirty(true)
	{ }
	Component(Entity* owner, const ComponentType type)
		: Insight::UUID(), m_owner(owner), m_type(type)
		, Insight::Serialization::Serializable(this, true)
		, m_isDirty(true)
	{ }
	virtual ~Component() {}

	void SetEntity(Entity* entity) { m_owner = entity; }
	Entity* GetEntity() const { return m_owner; }

	const bool& IsDirty() const { return m_isDirty; }

	virtual void OnUpdate(const float& deltaTime) { }

protected:
	bool m_isDirty;

private:
	void Clean() { m_isDirty = false; }

private:
	Entity* m_owner;
	ComponentType m_type;
	std::string m_uuid;

	friend Insight::Scene;
};
