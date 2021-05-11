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
}

void ComponentManager::Update(const float& deltaTime)
{
	IS_PROFILE_FUNCTION();
	for (auto& componentArray : m_componentArrays)
	{
		componentArray.second->Update(deltaTime);
	}
}
