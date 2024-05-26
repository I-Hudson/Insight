#pragma once

#include "Runtime/Defines.h"
#include "Core/IObject.h"

#include "Asset/Asset.h"

#include "ECS/Entity.h"
#include "ECS/EntityManager.h"

#include "World/Generated/World_reflect_generated.h"

#include <Reflect/Reflect.h>

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
		/// A World contains all entities and required date for it self to be processed.
		/// </summary>
		REFLECT_CLASS();
		class IS_RUNTIME World : public Asset
		{
			REFLECT_GENERATED_BODY();
		public:
			World();
			World(std::string worldName);
			World(const AssetInfo* assetInfo);
			~World();

			IS_SERIALISABLE_H(World);

			constexpr static const char* c_FileExtension = ".isworld";
			//REGISTER_RESOURCE(World);

			void Initialise();
			void Destroy();

			void EarlyUpdate();
			void Update(const float deltaTime);
			void LateUpdate();

			void SetWorldName(std::string worldName);
			std::string_view GetWorldName() const { return m_worldName; }

			void SetWorldState(WorldStates state);
			WorldStates GetWorldState() const;

			WorldTypes GetWorldType() const;

			bool IsPersistentScene() const { return m_persistentScene; }

			void SetOnlySearchable(bool onlySearchable);
			bool IsOnlySearchable() const { return m_onlySearchable; }

			Ptr<ECS::Entity> AddEntity();
			Ptr<ECS::Entity> AddEntity(std::string entity_name);
			Ptr<ECS::Entity> GetEntityByName(std::string entity_name) const;
			void RemoveEntity(Ptr<ECS::Entity>& entity);

			std::vector<Ptr<ECS::Entity>> GetAllEntitiesWithComponentByName(std::string_view component_type) const;
			/// @brief This only returns the roots entities.
			/// @return 
			std::vector<Ptr<ECS::Entity>> GetAllEntities() const;
			/// @brief Return all entities within the world. Including root and their children recursive.
			/// @return 
			std::vector<Ptr<ECS::Entity>> GetAllEntitiesFlatten() const;
			/// @brief Return the total number of entities within the world.
			/// @return u32
			u32 GetEntityCount() const;

			ECS::Entity* GetEntityByGUID(const Core::GUID& guid) const;
			
			void SaveWorld(std::string_view filePath) const;
			/// @brief Save the world to a file in a debug format (json) for readability.
			/// @param filePath 
			void SaveDebugWorld(std::string_view filePath) const;
			

			// Asset - Begin
		protected:
			virtual void OnUnload() override final;
			// Asset - End

		private:
			void AddEntityAndChildrenToVector(Ptr<ECS::Entity> const& entity, std::vector<Ptr<ECS::Entity>>& vector) const;

		private:
			/// Store all entites 
			std::string m_worldName = "";
			WorldStates m_worldState = WorldStates::Running;
			WorldTypes m_worldType = WorldTypes::Game;

			std::vector<Core::GUID> m_root_entities_guids;
			ECS::EntityManager m_entityManager;

			// Is this scene persistent. If 'true' then the scene can not be unloaded even if asked. The scene must be deleted to be removed. 
			bool m_persistentScene = false;
			// Can this scene only be found if searched for. This stops 'GetActiveScene' returning the scenes which might want to stay "hidden".
			bool m_onlySearchable = false;

			friend class WorldSystem;
		};
	}

	OBJECT_SERIALISER(Runtime::World, 4, 
		SERIALISE_PROPERTY(std::string, m_worldName, 1, 0)
		SERIALISE_PROPERTY(Runtime::WorldStates, m_worldState, 2, 0)
		SERIALISE_PROPERTY(Runtime::WorldTypes, m_worldType, 2, 0)
	
		SERIALISE_VECTOR_PROPERTY(Core::GUID, m_root_entities_guids, 1, 0)
		SERIALISE_OBJECT(ECS::EntityManager, m_entityManager, 1, 0)
		SERIALISE_PROPERTY_REMOVED(Core::GUID, m_cameraEntity, 1, 2)
	
		SERIALISE_PROPERTY(bool, m_persistentScene, 1, 0)
		SERIALISE_PROPERTY(bool, m_onlySearchable, 1, 0)
	
		SERIALISE_BASE(Runtime::Asset, 4, 0)
	);
}