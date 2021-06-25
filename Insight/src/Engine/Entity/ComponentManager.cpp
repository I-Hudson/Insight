
#include "Engine/Entity/ComponentManager.h"

#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Component/DirectionalLightComponent.h"
#include "Engine/Component/AnimatorComponent.h"
#include "Engine/Component/SkinnedMeshComponent.h"

ComponentManager::ComponentManager()
{
	RegisterComponent<AnimatorComponent>();
	RegisterComponent<CameraComponent>();
	RegisterComponent<TransformComponent, TransformComponentData>();
	RegisterComponent<DirectionalLightComponent, DirectionalLightComponentData>();

	RegisterComponent<MeshComponent>();
	RegisterComponent<SkinnedMeshComponent, SkinnedMeshComponentData>();
}

ComponentManager::~ComponentManager()
{
	for (auto& kvp : m_componentArrays)
	{
		kvp.second->OnDestroy();
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

void ComponentManager::OnBeginPlay()
{
	for (auto& componentArray : m_componentArrays)
	{
		componentArray.second->OnBeginPlay();
	}
}

void ComponentManager::OnEndPlay()
{
	for (auto& componentArray : m_componentArrays)
	{
		componentArray.second->OnEndPlay();
	}
}

std::vector<Component> ComponentManager::GetAllComponents(const EntityID& entity)
{
	std::vector<Component> components;
	for (auto& componentArray : m_componentArrays)
	{
		if (componentArray.second->HasComponent(entity))
		{
			components.push_back(componentArray.second->GetComponentBaseRef(entity));
		}
	}
	return components;
}
