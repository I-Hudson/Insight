#pragma once

#include "ECS/Entity.h"
#include "ECS/ComponentHandle.h"

#include <vector>
#include <queue>
#include <shared_mutex>

namespace Insight
{
	namespace ECS
	{
		class ECSWorld;

		// THREAD_SAFE

		class IS_ECS EntityManager
		{
		public:
			EntityManager();
			EntityManager(ECSWorld* ecsWorld);

			Entity AddNewEntity();
			void RemoveEntity(Entity& entity);

			void AddComponentToEntity(Entity entity, ComponentHandle handle);
			void RemoveComponentFromEntity(Entity entity, ComponentHandle handle);
			bool EntityHasComponent(Entity entity, ComponentHandle handle) const;

			EntityData GetEntityData(Entity entity) const;
			EntityData& GetEntityData(Entity entity);

		private:
			ECSWorld* m_ecsWorld = nullptr;
			std::vector<EntityData> m_entities;
			std::queue<int> m_freeEntities;

			std::shared_mutex m_lock;
		};
	}
}