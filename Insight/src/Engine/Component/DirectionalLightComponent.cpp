#include "ispch.h"
#include "Engine/Component/DirectionalLightComponent.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Component/TransformComponent.h"

DirectionalLightComponent::DirectionalLightComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity)
	: Component(componentManager, componentID, componentType, entityManager, entity)
{
	SetType<DirectionalLightComponent>();
}

DirectionalLightComponent::~DirectionalLightComponent()
{ }

void DirectionalLightComponent::OnUpdate(const float& a_deltaTime)
{
	DirectionalLightComponentData& data = GetComponentData<DirectionalLightComponentData>();
	data.Direction = glm::normalize(glm::vec3(0, 0, 0) - GetEntity().GetComponent<TransformComponent>().GetPostion());
}
