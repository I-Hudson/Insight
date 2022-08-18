#pragma once

namespace Insight
{
	namespace ECS
	{
		class ECSWorld;

		struct ECSSystemOnAddedData
		{
			ECSSystemOnAddedData(ECSWorld* const ecsWorld)
				: ECSWorld(ecsWorld)
			{ }
			ECSWorld* const ECSWorld;
		};
		template<typename Component>
		struct ECSSystemOnAdded
		{
			static void OnAdded(ECSSystemOnAddedData data, Component& component)
			{

			}
		};

		struct ECSSystemOnUpdateData
		{
			ECSSystemOnUpdateData(ECSWorld *const ecsWorld, float deltaTime)
				: ECSWorld(ecsWorld), DeltaTime(deltaTime)
			{ }
			ECSWorld * const ECSWorld;
			float DeltaTime;
		};
		template<typename Component>
		struct ECSSystemOnUpdate
		{
			static void OnUpdate(ECSSystemOnUpdateData data, Component& component)
			{

			}
		};
	}
}