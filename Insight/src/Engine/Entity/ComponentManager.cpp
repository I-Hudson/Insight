#include "ispch.h"
#include "Engine/Entity/ComponentManager.h"

#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/MeshComponent.h"

ComponentManager::ComponentManager()
{
	RegisterComponent<CameraComponent>();
	RegisterComponent<TransformComponent, TransformComponentData>();
	RegisterComponent<MeshComponent>();
}

ComponentManager::~ComponentManager()
{
	for (auto& kvp : m_componentArrays)
	{
		::Delete(kvp.second);
	}

	for (auto& kvp : m_componentDataArrays)
	{
		::Delete(kvp.second);
	}
}

void ComponentManager::Update(const float& deltaTime)
{
	IS_PROFILE_FUNCTION();
	for (auto& componentArray : m_componentArrays)
	{
		componentArray.second->Update(deltaTime);
	}
}

std::vector<Component> ComponentManager::GetAllComponents(const EntityID& entity)
{
	return std::vector<Component>();
}
