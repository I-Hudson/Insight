#include "World/World.h"

#include "Core/Profiler.h"
#include "Core/Logger.h"

#include "Event/EventSystem.h"
#include "Runtime/RuntimeEvents.h"
#include "Serialisation/Archive.h"
#include "Core/Compression.h"

namespace Insight
{
	namespace Runtime
	{
		World::World()
			: IResource("Default")
			, m_worldName("Default")
		{
			m_entityManager.SetWorld(this);
		}

		World::World(std::string_view filePath, std::string worldName)
			: IResource(filePath)
			, m_worldName(std::move(worldName))
		{
			m_entityManager.SetWorld(this);
		}

		World::~World()
		{ }

		void World::Initialise()
		{
		}

		void World::Destroy()
		{
			Core::EventSystem::Instance().DispatchEventNow(MakeRPtr<WorldDestroyEvent>(this));

			m_worldName = "Default";
			m_worldState = WorldStates::Paused;
			m_worldType = WorldTypes::Game;
			m_root_entities_guids.clear();
			m_entityManager.Destroy();

			m_persistentScene = false;
			m_onlySearchable = false;
		}

		void World::EarlyUpdate()
		{
			IS_PROFILE_FUNCTION();
			if (m_worldState == WorldStates::Paused)
			{
				return;
			}
			m_entityManager.EarlyUpdate();
		}

		void World::Update(const float deltaTime)
		{
			IS_PROFILE_FUNCTION();
			if (m_worldState == WorldStates::Paused)
			{
				return;
			}
			m_entityManager.Update(deltaTime);
		}

		void World::LateUpdate()
		{
			IS_PROFILE_FUNCTION();
			if (m_worldState == WorldStates::Paused)
			{
				return;
			}
			m_entityManager.LateUpdate();
		}

		void World::SetWorldName(std::string worldName)
		{
			ASSERT(!worldName.empty()); 
			m_worldName = worldName;
		}

		void World::SetWorldState(WorldStates state)
		{
			m_worldState = state;
		}

		WorldStates World::GetWorldState() const
		{
			return m_worldState;
		}

		WorldTypes World::GetWorldType() const
		{
			return m_worldType;
		}

		void World::SetOnlySearchable(bool onlySearchable)
		{
			m_onlySearchable = onlySearchable;
		}

		Ptr<ECS::Entity> World::AddEntity()
		{
			return AddEntity("Entity");
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

		ECS::Entity* World::GetEntityByGUID(const Core::GUID& guid) const
		{
			return m_entityManager.GetEntityByGUID(guid);
		}

		void World::SaveWorld(std::string_view filePath) const
		{
			Runtime::World::ResourceSerialiserType serialiser(false);
			RemoveConst(this)->Serialise(&serialiser);

			std::vector<Byte> worldData = serialiser.GetSerialisedData();
			FileSystem::SaveToFile(worldData, filePath, FileType::Binary, true);
		}

		void World::SaveDebugWorld(std::string_view filePath) const
		{
			Serialisation::JsonSerialiser serialiser(false);
			RemoveConst(this)->Serialise(&serialiser);

			std::vector<Byte> worldData = serialiser.GetSerialisedData();
			FileSystem::SaveToFile(worldData, filePath, FileType::Text, true);
		}

		void World::Serialise(Serialisation::ISerialiser* serialiser)
		{
			Core::EventSystem::Instance().DispatchEventNow(MakeRPtr<WorldSaveEvent>(this));

			Serialisation::SerialiserObject<World> serialiserObject;
			serialiserObject.Serialise(serialiser, *this);
		}

		void World::Deserialise(Serialisation::ISerialiser* serialiser)
		{
			Destroy();

			Core::EventSystem::Instance().DispatchEventNow(MakeRPtr<WorldLoadEvent>(this));
			Serialisation::SerialiserObject<World> serialiserObject;
			serialiserObject.Deserialise(serialiser, *this);
			m_entityManager.SetWorld(this);
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