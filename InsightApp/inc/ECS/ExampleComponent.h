#pragma once

#include "ECS/ECS.h"

#ifdef IS_EXP_ECS
namespace Insight
{
	namespace App
	{
		struct ExampleComponent
		{
			float floatData;
			int intData;
		};

		template<>
		class ECSSystem<ExampleComponent> : public IECSSystem
		{
		public:
			virtual void Update(IECSComponentArray* componentArray, float deltaTime) override
			{
				ECSComponentArray<ExampleComponent>* compoenentArray = static_cast<ECSComponentArray<ExampleComponent>*>(componentArray);
				for (size_t i = 0; i < compoenentArray->m_componentData.size(); ++i)
				{
					ExampleComponent& component = compoenentArray->m_componentData[i];
				}
			}
		};
	}
}
#endif //#ifdef IS_EXP_ECS