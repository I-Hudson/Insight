#pragma once

#include "Runtime/Defines.h"
#include "Core/IObject.h"

#include "ECS/Entity.h"
#include "ECS/EntityManager.h"

#include <string>

namespace Insight
{
	namespace Runtime
	{
		class WorldSystem;

		enum class WorldStates
		{
			Paused,			// World is paused, nothing is happening.
			Running,		// Current world is running, things are being updated.
		};

		enum class WorldTypes
		{
			Game,	// World is for game use (default).
			Tools	// World is for tool/development use (E.G: Editor).
		};

		/// <summary>
		/// Describes a single "world" within the engine.
		/// A World contains all entities and requried date for it self to be processed.
		/// </summary>
		class IS_RUNTIME World : public IObject
		{
		public:
			World();
			World(std::string worldName);
			World(World const& other) = delete;
			World(World&& other);
			~World();

			void Destroy();

			void EarlyUpdate();
			void Update(const float deltaTime);
			void LateUpdate();

			void SetWorldName(std::string worldName) { m_worldName = worldName; }
			std::string_view GetWorldName() const { return m_worldName; }

			bool IsPersistentScene() const { return m_persistentScene; }

			void SetOnlySearchable(bool onlySearchable);
			bool IsOnlySearchable() const { return m_onlySearchable; }

			Ptr<ECS::Entity> AddEntity();
			Ptr<ECS::Entity> AddEntity(std::string entity_name);
			Ptr<ECS::Entity> GetEntityByName(std::string entity_name) const;
			void RemoveEntity(Ptr<ECS::Entity>& entity);

			std::vector<Ptr<ECS::Entity>> GetAllEntitiesWithComponentByName(std::string_view component_type) const;
			/// <summary>
			/// This only returns the roots entities.
			/// </summary>
			/// <returns></returns>
			std::vector<Ptr<ECS::Entity>> GetAllEntities() const;

		private:
			/// Store all entites 
			std::string m_worldName = "";
			WorldStates m_worldState = WorldStates::Paused;
			WorldTypes m_worldType = WorldTypes::Game;

			std::vector<Ptr<ECS::Entity>> m_root_entities;
			ECS::EntityManager m_entityManager;
			Ptr<ECS::Entity> m_cameraEntity = nullptr;

			// Is this scene persistent. If 'true' then the scene can not be unloaded even if asked. The scene must be deleted to be removed. 
			bool m_persistentScene = false;
			// Can this scene only be found if searched for. This stops 'GetActiveScene' returning the scenes which might want to stay "hidden".
			bool m_onlySearchable = false;

			friend class WorldSystem;
		};
	}
}