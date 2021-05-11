#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Serialization/Serializable.h"

class Entity;

class IS_API Component
{
public:
	Component();
	Component(EntityID entity);
	virtual ~Component();
	virtual void OnUpdate(const float& a_deltaTime) = 0;
	Entity GetEntity();

protected:
	EntityID m_entityID;
};
