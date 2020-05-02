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

class IS_API Component
{
public:
	Component(Entity* owner, const ComponentType type)
		: m_owner(owner), m_type(type)
	{
		m_uuid = Insight::GenUUID();
	}
	virtual ~Component() {}

	const std::string& GetUUID() const { return m_uuid; }

private:
	Entity* m_owner;
	ComponentType m_type;
	std::string m_uuid;
};
