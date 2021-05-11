#include "ispch.h"
#include "Engine/Component/Component.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Scene/Scene.h"

Component::Component()
	: m_entityID(-1)
{ }

Component::Component(EntityID entity)
	: m_entityID(entity) 
{ }

Component::~Component()
{ }

Entity Component::GetEntity()
{
	return Scene::ActiveScene()->GetEntity(m_entityID);
}
