#pragma once

#include "Insight/Core.h"
#include "Insight/UUID.h"

class Entity;

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

class IS_API Component : public Insight::UUID
{
public:
	Component(Entity* owner, const ComponentType type)
		: Insight::UUID(), m_owner(owner), m_type(type)
	{ }
	virtual ~Component() {}

	const Entity* GetEntity() const { return m_owner; }

private:
	Entity* m_owner;
	ComponentType m_type;
	std::string m_uuid;
};
