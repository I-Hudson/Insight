#include "ECS/EntityManager.h"

#include "Core/Logger.h"

namespace Insight
{
	namespace ECS
	{
		Entity EntityManager::AddNewEntity()
		{
			if (m_freeEntities.size() == 0)
			{
				const int entitiesSize = static_cast<int>(m_entities.size());
				m_entities.push_back(EntityData());
				m_freeEntities.push(entitiesSize);
			}

			const int freeEntity = m_freeEntities.front();
			m_freeEntities.pop();

			Entity e;
			e.m_id = freeEntity;
			e.m_entityManager = this;
			return e;
		}

		void EntityManager::RemoveEntity(Entity& entity)
		{
			if (!entity.IsVaild())
			{
				IS_CORE_ERROR("[EntityManager::RemoveEntity] Entity '{}', is invalid.", entity.GetId());
				return;
			}

			m_freeEntities.push(entity.GetId());
			m_entities.at(entity.GetId()) = { };

			entity = { };
		}

		EntityData EntityManager::GetEntityData(Entity entity) const
		{
			if (!entity.IsVaild())
			{
				IS_CORE_ERROR("[EntityManager::RemoveEntity] Entity '{}', is invalid.", entity.GetId());
				return EntityData();
			}

			return m_entities.at(entity.GetId());
		}

		EntityData& EntityManager::GetEntityData(Entity entity)
		{
			if (!entity.IsVaild())
			{
				IS_CORE_ERROR("[EntityManager::RemoveEntity] Entity '{}', is invalid.", entity.GetId());
				return EntityData();
			}

			return m_entities.at(entity.GetId());
		}
	}
}