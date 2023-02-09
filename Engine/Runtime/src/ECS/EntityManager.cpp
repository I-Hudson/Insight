#include "ECS/EntityManager.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/TagComponent.h"

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
		{
			ComponentRegistry::RegisterInternalComponents();
		}

		EntityManager::EntityManager(EntityManager&& other)
		{
			*(this) = std::move(other);
		}

		EntityManager& EntityManager::operator=(EntityManager&& other)
		{
			ComponentRegistry::RegisterInternalComponents();

#ifdef ECS_ENABLED
			m_ecsWorld = other.m_ecsWorld;
			other.m_ecsWorld = nullptr;
#else
			m_world = other.m_world;
			other.m_world = nullptr;
#endif
			std::for_each(other.m_entities.begin(), other.m_entities.end(), [this](UPtr<Entity>& entity)
				{
					m_entities.push_back(std::move(entity));
				});
			other.m_entities.clear();
			return *this;
		}

#ifdef ECS_ENABLED
		EntityManager::EntityManager(ECSWorld* ecsWorld)
			: m_ecsWorld(ecsWorld)
		{ }
#else
		void EntityManager::SetWorld(Runtime::World* world)
		{
			m_world = world;
		}
#endif

		Entity* EntityManager::AddNewEntity()
		{
			return AddNewEntity("Entity");
		}

		Entity* EntityManager::AddNewEntity(std::string entity_name)
		{
			IS_PROFILE_FUNCTION();
#ifdef ECS_ENABLED
			UPtr<Entity> new_entity = MakeUPtr<Entity>(m_ecsWorld, entity_name);
#else
			UPtr<Entity> new_entity = MakeUPtr<Entity>(this, entity_name);

#endif
			new_entity->AddComponentByName(TransformComponent::Type_Name);
			new_entity->AddComponentByName(TagComponent::Type_Name);

			{
				std::lock_guard lock(m_lock);
				m_entities.emplace_back(std::move(new_entity));
			}
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

		void EntityManager::EarlyUpdate()
		{
			for (UPtr<Entity>& entity : m_entities)
			{
				entity->EarlyUpdate();
			}
		}

		void EntityManager::Update(const float delta_time)
		{
			IS_PROFILE_FUNCTION();
			for (UPtr<Entity>& entity : m_entities)
			{
				entity->Update(delta_time);
			}
		}

		void EntityManager::LateUpdate()
		{
			IS_PROFILE_FUNCTION();
			for (UPtr<Entity>& entity : m_entities)
			{
				entity->LateUpdate();
			}
		}

		void EntityManager::Destroy()
		{
			IS_PROFILE_FUNCTION();
			for (UPtr<Entity>& e :  m_entities)
			{
				e->Destroy(); 
				e.Reset();
			}
			m_entities.resize(0);
		}

		Ptr<Entity> EntityManager::GetEntityByName(std::string_view entity_name) const
		{
			IS_PROFILE_FUNCTION();
			for (const UPtr<Entity>& e : m_entities)
			{
				if (e->GetName() == entity_name) 
				{
					return e;
				}
			}
			return nullptr;
		}

		std::vector<Ptr<ECS::Entity>> EntityManager::GetAllEntitiesWithComponentByName(std::string_view component_type) const
		{
			IS_PROFILE_FUNCTION();

			std::vector<Ptr<ECS::Entity>> entities;
			for (const UPtr<Entity>& e : m_entities)
			{
				if (e->GetComponentByName(component_type) != nullptr)
				{
					entities.push_back(e);
				}
			}
			return entities;
		}

		std::vector<Ptr<ECS::Entity>> EntityManager::GetAllEntities() const
		{
			std::vector<Ptr<ECS::Entity>> entities;
			entities.reserve(m_entities.size());
			for (const UPtr<Entity>& e : m_entities)
			{
				entities.push_back(e);
			}
			return entities;
		}

		ECS::Entity* EntityManager::GetEntityByGUID(Core::GUID guid) const
		{
			for (const UPtr<Entity>& e : m_entities)
			{
				if (e->GetGUID() == guid)
				{
					return e.Get();
				}
			}
			return nullptr;
		}

		Entity* EntityManager::AddNewEntity(Core::GUID guid)
		{
			Entity* e = AddNewEntity();
			e->m_guid = guid;
			return e;
		}

		Entity* EntityManager::AddNewEntity(std::string entity_name, Core::GUID guid)
		{
			Entity* e = AddNewEntity(entity_name);
			e->m_guid = guid;
			return e;
		}

		Component* EntityManager::AddComponentToEntity(Core::GUID entityGuid, Core::GUID componentGuid, std::string componentTypeName)
		{
			Entity* e = GetEntityByGUID(entityGuid);
			if (e == nullptr)
			{
				IS_CORE_ERROR("[EntityManager::AddComponentToEntity] Unable to find entity with guid '{}'.", entityGuid.ToString());
				return nullptr;
			}

			ECS::Component* component = e->AddComponentByName(componentTypeName);
			if (component == nullptr)
			{
				IS_CORE_ERROR("[EntityManager::AddComponentToEntity] Unable to add component with typename '{}'.", componentTypeName);
				return nullptr;
			}
			component->m_guid = componentGuid;
			return component;
		}

		IS_SERIALISABLE_CPP(EntityManager)
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