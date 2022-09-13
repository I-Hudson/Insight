#include "ECS/EntityManager.h"

#include "Core/Logger.h"

namespace Insight
{
	namespace ECS
	{
#ifdef IS_ECS_ENABLED
		EntityManager::EntityManager()
		{ }

		EntityManager::EntityManager(ECSWorld* ecsWorld)
			: m_ecsWorld(ecsWorld)
		{ }

		Entity EntityManager::AddNewEntity()
		{
			int freeEntity = -1;
			{
				std::lock_guard lock(m_lock);

				if (m_freeEntities.size() == 0)
				{
					const int entitiesSize = static_cast<int>(m_entities.size());
					m_entities.push_back(EntityData());
					m_freeEntities.push(entitiesSize);
				}

				freeEntity = m_freeEntities.front();
				m_freeEntities.pop();
			}
			Entity e;
			e.m_id = freeEntity;
			e.m_entityManager = this;
			e.m_ecsWorld = m_ecsWorld;

			{
				EntityData& data = GetEntityData(e);
				data.GUID.GetNewGUID();
			}

			return e;
		}

		void EntityManager::RemoveEntity(Entity& entity)
		{
			std::lock_guard lock(m_lock);

			if (!entity.IsVaild())
			{
				IS_CORE_ERROR("[EntityManager::RemoveEntity] Entity '{}', is invalid.", entity.GetId());
				return;
			}

			m_freeEntities.push(entity.GetId());
			m_entities.at(entity.GetId()) = { };

			entity = { };
		}

		void EntityManager::AddComponentToEntity(Entity entity, ComponentHandle handle)
		{
			if (!entity.IsVaild())
			{
				IS_CORE_ERROR("[EntityManager::AddComponentToEntity] Entity '{}', is invalid.", entity.GetId());
				return;
			}

			if (EntityHasComponent(entity, handle))
			{
				return;
			}

			EntityData& data = GetEntityData(entity);
			std::lock_guard lock(m_lock);
			data.Components[handle.GetType()].insert(handle.GetIndex());
		}

		void EntityManager::RemoveComponentFromEntity(Entity entity, ComponentHandle handle)
		{
			if (!entity.IsVaild())
			{
				IS_CORE_ERROR("[EntityManager::RemoveComponentFromEntity] Entity '{}', is invalid.", entity.GetId());
				return;
			}

			if (!EntityHasComponent(entity, handle))
			{
				return;
			}
			{
				EntityData& data = GetEntityData(entity);
				std::lock_guard lock(m_lock);
				data.Components[handle.GetType()].erase(handle.GetIndex());
			}
		}

		bool EntityManager::EntityHasComponent(Entity entity, ComponentHandle handle) const
		{
			if (!entity.IsVaild())
			{
				IS_CORE_ERROR("[EntityManager::EntityHasComponent] Entity '{}', is invalid.", entity.GetId());
				return false;
			}
			const EntityData& data = GetEntityData(entity);

			std::shared_lock lock(const_cast<std::shared_mutex&>(m_lock));
			auto componentSet = data.Components.find(handle.GetType());
			if (componentSet == data.Components.end()
				|| componentSet->second.find(handle.GetIndex()) == componentSet->second.end())
			{
				return false;
			}

			return true;
		}

		EntityData EntityManager::GetEntityData(Entity entity) const
		{
			std::shared_lock lock(const_cast<std::shared_mutex&>(m_lock));

			if (!entity.IsVaild())
			{
				IS_CORE_ERROR("[EntityManager::RemoveEntity] Entity '{}', is invalid.", entity.GetId());
				return EntityData();
			}

			return m_entities.at(entity.GetId());
		}

		EntityData& EntityManager::GetEntityData(Entity entity)
		{
			std::shared_lock lock(m_lock);

			if (!entity.IsVaild())
			{
				IS_CORE_ERROR("[EntityManager::RemoveEntity] Entity '{}', is invalid.", entity.GetId());
				return EntityData();
			}

			return m_entities.at(entity.GetId());
		}
#else

		EntityManager::EntityManager()
		{ }

		EntityManager::EntityManager(ECSWorld* ecsWorld)
			: m_ecsWorld(ecsWorld)
		{ }

		Entity* EntityManager::AddNewEntity()
		{
			return AddNewEntity("Entity");
		}

		Entity* EntityManager::AddNewEntity(std::string entity_name)
		{
			UPtr<Entity> new_entity = MakeUPtr<Entity>(entity_name);
			m_entities.emplace_back(std::move(new_entity));
			return m_entities.back().Get();
		}

		void EntityManager::RemoveEntity(Entity*& entity)
		{
			std::lock_guard lock(m_lock);

			u32 index = 0;
			for (UPtr<Entity>& e : m_entities)
			{
				if (e == entity)
				{
					e.Reset();
					break;
				}
				++index;
			}
			ASSERT(entity == nullptr);
			m_entities.erase(m_entities.begin() + index);
		}

		void EntityManager::Update(const float delta_time)
		{
			for (UPtr<Entity>& entity : m_entities)
			{
				entity->Update(delta_time);
			}
		}
#endif
	}
}
#ifdef TESTING
#include "doctest.h"
TEST_SUITE("Entity Manager")
{
	using namespace Insight::ECS;

	EntityManager entityManager;
	Entity e;
	TEST_CASE("Add entity")
	{
		e = entityManager.AddNewEntity();
		CHECK(e.IsVaild());
	}

	TEST_CASE("Entity set name")
	{
		CHECK(e.GetName() == "");
		const char* testName = "TestName";
		e.SetName(testName);
		CHECK(e.GetName() == testName);
	}

	TEST_CASE("Remove entity")
	{
		CHECK(e.IsVaild());
		entityManager.RemoveEntity(e);
		CHECK(!e.IsVaild());
	}
}
#endif