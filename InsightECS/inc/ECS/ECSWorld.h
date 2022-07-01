#pragma once

#include "ECS/Defines.h"
#include "ECS/EntityManager.h"

namespace Insight
{
	namespace ECS
	{
		/// <summary>
		/// World manager for all ECS objects (entities and components).
		/// This should have creation, deletion, and updating all objects.
		/// </summary>
		class IS_ECS ECSWorld
		{
		public:
			~ECSWorld();

			void Update(float deltaTime);

			Entity AddEntity();
			void RemoveEntity(Entity& entity);

		private:
			EntityManager m_entityManager;
		};
	}
}