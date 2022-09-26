#include "ECS/ComponentArray.h"

#include "Core/Profiler.h"

namespace Insight
{
	namespace ECS
	{
#ifdef IS_ECS_ENABLED
		ComponentArrayManager::ComponentArrayManager(ECSWorld* const ecsWorld)
			: m_ecsWorld(ecsWorld)
		{ }

		void ComponentArrayManager::Update(float deltaTime)
		{
			IS_PROFILE_FUNCTION();
			for (const auto& pair : m_componentArrays)
			{
				IS_PROFILE_SCOPE("Single component array");
				pair.second->Update(deltaTime);
			}
		}

		void ComponentArrayManager::RemoveComponent(Entity entity, ComponentHandle& handle)
		{
			IComponentArray* componentArray = GetComponentArray(handle.GetType());
			if (componentArray == nullptr)
			{
				return;
			}
			componentArray->RemoveComponent(entity, handle);
		}
#endif
	}
}