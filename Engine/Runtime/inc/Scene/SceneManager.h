#pragma once

#include "Core/Singleton.h"
#include "Core/Memory.h"

#include "ECS/ECSWorld.h"

#include <vector>

namespace Insight
{
	namespace App
	{
		class SceneManager;

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

		class IS_RUNTIME Scene
		{
		public:
			Scene();
			Scene(std::string sceneName);
			Scene(Scene const& other) = delete;
			Scene(Scene&& other);
			~Scene();

			void Destroy();

			void EarlyUpdate();
			void Update(const float deltaTime);
			void LateUpdate();

			void SetSceneName(std::string sceneName) { m_sceneName = m_sceneName; }
			std::string_view GetSceneName() const { return m_sceneName; }

			bool IsPersistentScene() const { return m_persistentScene; }

			void SetOnlySearchable(bool onlySearchable);
			bool IsOnlySearchable() const { return m_onlySearchable; }

#ifdef ECS_ENABLED
			Ptr<ECS::ECSWorld> GetECSWorld() const { return m_ecsWorld; }
#else
			Ptr<ECS::Entity> AddEntity();
			Ptr<ECS::Entity> AddEntity(std::string entity_name);
			Ptr<ECS::Entity> GetEntityByName(std::string entity_name) const;
			void RemoveEntity(Ptr<ECS::Entity>& entity);
#endif
			std::vector<Ptr<ECS::Entity>> GetAllEntitiesWithComponentByName(std::string_view component_type) const;
			/// <summary>
			/// This only returns the roots entities.
			/// </summary>
			/// <returns></returns>
			std::vector<Ptr<ECS::Entity>> GetAllEntities() const;

		private:
			/// Store all entites 
			std::string m_sceneName = "";
			WorldStates m_worldState = WorldStates::Paused;
			WorldTypes m_worldType = WorldTypes::Game;

#ifdef ECS_ENABLED
			UPtr<ECS::ECSWorld> m_ecsWorld = nullptr;
#else
			//
			std::vector<Ptr<ECS::Entity>> m_root_entities;
			ECS::EntityManager m_entityManager;
#endif
			Ptr<ECS::Entity> m_cameraEntity = nullptr;

			// Is this scene persistent. If 'true' then the scene can not be unloaded even if asked. The scene must be deleted to be removed. 
			bool m_persistentScene = false;
			// Can this scene only be found if searched for. This stops 'GetActiveScene' returning the scenes which might want to stay "hidden".
			bool m_onlySearchable = false;

			friend class SceneManager;
		};

		class IS_RUNTIME SceneManager : public Core::Singleton<SceneManager>
		{
		public:
			SceneManager();
			virtual ~SceneManager();

			void EarlyUpdate();
			void Update(const float deltaTime);
			void LateUpdate();

			void Destroy();

			WPtr<Scene> CreateScene(std::string sceneName = "", WorldTypes worldType = WorldTypes::Game);
			// Create a new scene which can not be unloaded. This scene will alway be loaded.
			WPtr<Scene> CreatePersistentScene(std::string sceneName = "", WorldTypes worldType = WorldTypes::Game);

			// Set a single scene as active.
			void SetActiveScene(WPtr<Scene> scene);
			// Add a scene to be active.
			void AddActiveScene(WPtr<Scene> scene);
			// Remove a scene from being active.
			void RemoveScene(WPtr<Scene> scene);

			WPtr<Scene> GetActiveScene() const;
			WPtr<Scene> FindSceneByName(std::string_view sceneName);
			WPtr<Scene> GetSceneFromIndex(u32 index) const;
			std::vector<WPtr<Scene>> GetAllScenes() const;

		private:
			std::vector<RPtr<Scene>> m_activeScenes;
			std::vector<RPtr<Scene>> m_scenes;
		};
	}
}