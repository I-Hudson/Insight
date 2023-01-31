#include "World/World.h"

#include "Core/Profiler.h"
#include "Core/Logger.h"

namespace Insight
{
	namespace Runtime
	{
		World::World()
			: m_worldName("Default")
		{
			m_entityManager.SetWorld(this);
		}

		World::World(std::string worldName)
			: m_worldName(std::move(worldName))
		{
			m_entityManager.SetWorld(this);
		}

		World::~World()
		{

		}

		void World::Destroy()
		{ }

		void World::EarlyUpdate()
		{
			IS_PROFILE_FUNCTION();
			if (m_worldType == WorldTypes::Game
				&& m_worldState == WorldStates::Paused)
			{
				return;
			}
			m_entityManager.EarlyUpdate();
		}

		void World::Update(const float deltaTime)
		{
			IS_PROFILE_FUNCTION();
			if (m_worldType == WorldTypes::Game 
				&& m_worldState == WorldStates::Paused)
			{
				return;
			}
			m_entityManager.Update(deltaTime);
		}

		void World::LateUpdate()
		{
			IS_PROFILE_FUNCTION();
			if (m_worldType == WorldTypes::Game
				&& m_worldState == WorldStates::Paused)
			{
				return;
			}
			m_entityManager.LateUpdate();
		}

		void World::SetOnlySearchable(bool onlySearchable)
		{
			m_onlySearchable = onlySearchable;
		}

		Ptr<ECS::Entity> World::AddEntity()
		{
			return AddEntity("");
		}

		Ptr<ECS::Entity> World::AddEntity(std::string entity_name)
		{
			Ptr<ECS::Entity> e = m_entityManager.AddNewEntity(entity_name);
			m_root_entities_guids.push_back(e->GetGUID());
			return e;
		}

		Ptr<ECS::Entity> World::GetEntityByName(std::string entity_name) const
		{
			return m_entityManager.GetEntityByName(entity_name);
		}

		void World::RemoveEntity(Ptr<ECS::Entity>& entity)
		{
			ECS::Entity* e = entity.Get();
			if (e)
			{
				Core::GUID guid = e->GetGUID();
				IS_UNUSED(std::remove_if(m_root_entities_guids.begin(), m_root_entities_guids.end(), [guid](const Core::GUID& otherGuid)
					{
						return guid == otherGuid;
					}));
				m_entityManager.RemoveEntity(e);
			}
		}

		std::vector<Ptr<ECS::Entity>> World::GetAllEntitiesWithComponentByName(std::string_view component_type) const
		{
			return m_entityManager.GetAllEntitiesWithComponentByName(component_type);
		}

		std::vector<Ptr<ECS::Entity>> World::GetAllEntities() const
		{
			std::vector<Ptr<ECS::Entity>> entities;
			for (Core::GUID const& g : m_root_entities_guids)
			{
				entities.push_back(m_entityManager.GetEntityByGUID(g));
			}
			return entities;
		}

		std::vector<Ptr<ECS::Entity>> World::GetAllEntitiesFlatten() const
		{
			IS_PROFILE_FUNCTION();
			return m_entityManager.GetAllEntities();
		}

		ECS::Entity* World::GetEntityByGUID(Core::GUID guid) const
		{
			return m_entityManager.GetEntityByGUID(guid);
		}

		void World::AddEntityAndChildrenToVector(Ptr<ECS::Entity> const& entity, std::vector<Ptr<ECS::Entity>>& vector) const
		{
			vector.push_back(entity);
			for (u32 i = 0; i < entity->GetChildCount(); ++i)
			{
				AddEntityAndChildrenToVector(entity->GetChild(i), vector);
			}
		}
	}
}