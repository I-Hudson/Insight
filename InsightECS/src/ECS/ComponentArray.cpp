#include "ECS/ComponentArray.h"

namespace Insight
{
	namespace ECS
	{
		void ComponentArrayManager::RemoveComponent(Entity entity, ComponentHandle& handle)
		{
			IComponentArray* componentArray = GetComponentArray(handle.GetType());
			if (componentArray == nullptr)
			{
				return;
			}
			componentArray->RemoveComponent(entity, handle);
		}
	}
}