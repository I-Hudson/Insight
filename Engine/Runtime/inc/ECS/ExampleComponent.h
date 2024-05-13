#pragma once

#include "ECS/ECS.h"

#include <iostream>

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
		class ECSSystemImpl<ExampleComponent>
		{
		public:
			static void Update(ECS& ecs, Entity ownerEnttity, ExampleComponent& component, float deltaTime)
			{
				component.intData++;
			}
		};

		struct ExampleComponentPrint
		{ };
		template<>
		class ECSSystemImpl<ExampleComponentPrint>
		{
		public:
			static void Update(ECS& ecs, Entity ownerEnttity, ExampleComponentPrint& component, float deltaTime)
			{
				ExampleComponent com = ecs.GetComponent<ExampleComponent>(ownerEnttity);
				std::cout << "Example: " << com.intData;
			}
		};

		struct TransformComponent
		{
			Maths::Vector3 Position;
			glm::quat Qauternion;
			Maths::Vector3 Scale;
		};

		struct TagComponent
		{
			std::vector<std::string> Tags;
		};
	}
}
#endif ///#ifdef IS_EXP_ECS