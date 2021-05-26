#include "ispch.h"
#include "Engine/Component/DirectionalLightComponent.h"

DirectionalLightComponent::DirectionalLightComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity)
	: Component(componentManager, componentID, componentType, entityManager, entity)
{
	SetType<DirectionalLightComponent>();
}

DirectionalLightComponent::~DirectionalLightComponent()
{ }

void DirectionalLightComponent::OnUpdate(const float& a_deltaTime)
{ }
