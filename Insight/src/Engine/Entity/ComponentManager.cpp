#include "ispch.h"
#include "Engine/Entity/ComponentManager.h"

#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/MeshComponent.h"

ComponentManager::ComponentManager()
{
	RegisterComponent<CameraComponent>();
	RegisterComponent<TransformComponent>();
	RegisterComponent<MeshComponent>();
}

ComponentManager::~ComponentManager()
{
	for (auto& kvp : m_componentArrays)
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
