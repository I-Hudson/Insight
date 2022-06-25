#include "ECS/ECS.h"

#include "ECS/ExampleComponent.h"

#ifdef IS_EXP_ECS

namespace Insight
{
	namespace App
	{
		ECS::ECS()
		{
			RegisterSystem<ExampleComponent>();
		}

		void ECS::Update(float deltaTime)
		{
			for (size_t i = 0; i < m_componentSystemsToUpdate.size(); ++i)
			{
				const u64 componentId = m_componentSystemsToUpdate.at(i);
				IECSSystem* system = m_componentSystems.find(componentId)->second.Get();
				IECSComponentArray* componentArray = m_componentArrays.find(componentId)->second.Get();

				system->Update(componentArray, deltaTime);
			}
		}
	}
}
#endif //#ifdef IS_EXP_ECS