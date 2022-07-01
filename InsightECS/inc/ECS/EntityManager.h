#pragma once

#include "ECS/Entity.h"

#include <vector>
#include <queue>

namespace Insight
{
	namespace ECS
	{
		class IS_ECS EntityManager
		{
		public:

			Entity AddNewEntity();
			void RemoveEntity(Entity& entity);

			EntityData GetEntityData(Entity entity) const;
			EntityData& GetEntityData(Entity entity);

		private:
			std::vector<EntityData> m_entities;
			std::queue<int> m_freeEntities;
		};
	}
}