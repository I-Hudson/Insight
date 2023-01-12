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
			m_root_entities.push_back(e);
			return e;
		}

		Ptr<ECS::Entity> World::GetEntityByName(std::string entity_name) const
		{
			return m_entityManager.GetEntityByName(entity_name);
		}

		void World::RemoveEntity(Ptr<ECS::Entity>& entity)
		{
			ECS::Entity* e = entity.Get();
			IS_UNUSED(std::remove_if(m_root_entities.begin(), m_root_entities.end(), [e](const Ptr<ECS::Entity>& other_entity)
				{
					return e == other_entity.Get();
				}));
			m_entityManager.RemoveEntity(e);
		}

		std::vector<Ptr<ECS::Entity>> World::GetAllEntitiesWithComponentByName(std::string_view component_type) const
		{
			return m_entityManager.GetAllEntitiesWithComponentByName(component_type);
		}

		std::vector<Ptr<ECS::Entity>> World::GetAllEntities() const
		{
			return m_root_entities;
		}

		std::vector<Ptr<ECS::Entity>> World::GetAllEntitiesFlatten() const
		{
			std::vector<Ptr<ECS::Entity>> entities;
			for (size_t i = 0; i < m_root_entities.size(); ++i)
			{
				Ptr<ECS::Entity> const& entity = m_root_entities.at(i);
				AddEntityAndChildrenToVector(entity, entities);
			}
			return entities;
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