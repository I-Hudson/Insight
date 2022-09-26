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

#ifdef IS_ECS_ENABLED
		/// THREAD_SAFE

		class IS_RUNTIME EntityManager
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
#else

		class IS_RUNTIME EntityManager
		{
		public:
			EntityManager();
			EntityManager(ECSWorld* ecsWorld);

			Entity* AddNewEntity();
			Entity* AddNewEntity(std::string entity_name);
			void RemoveEntity(Entity*& entity);

			void EarlyUpdate();
			void Update(const float deltaTime);
			void LateUpdate();

			void Destroy();

			Ptr<Entity> GetEntityByName(std::string_view entity_name) const;
			std::vector<Ptr<ECS::Entity>> GetAllEntitiesWithComponentByName(std::string_view component_type) const;

		private:
			ECSWorld* m_ecsWorld = nullptr;
			std::vector<UPtr<Entity>> m_entities;

			std::shared_mutex m_lock;
		};

#endif
	}
}