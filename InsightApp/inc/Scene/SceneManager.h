#pragma once

#include "Core/Singleton.h"
#include "Core/Memory.h"

#include "ECS/ECSWorld.h"

#include <vector>

namespace Insight
{
	namespace App
	{
		class Scene
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

			void Render();

			void SetSceneName(std::string sceneName) { m_sceneName = m_sceneName; }
			std::string_view GetSceneName() const { return m_sceneName; }

			Ptr<ECS::ECSWorld> GetECSWorld() const { return m_ecsWorld; }

			std::vector<Ptr<ECS::Entity>> GetAllEntitiesWithComponentByName(std::string_view component_type) const;

		private:
			/// Store all entites 
			std::string m_sceneName = "";
			UPtr<ECS::ECSWorld> m_ecsWorld = nullptr;
		};

		class SceneManager : public Core::Singleton<SceneManager>
		{
		public:
			SceneManager();
			virtual ~SceneManager();

			void EarlyUpdate();
			void Update(const float deltaTime);
			void LateUpdate();

			void Destroy();

			WPtr<Scene> CreateScene(std::string sceneName = "");
			void SetActiveScene(WPtr<Scene> scene);
			void RemoveScene(WPtr<Scene> scene);

			WPtr<Scene> GetActiveScene() const { return m_activeScene; }
			WPtr<Scene> FindSceneByName(std::string_view sceneName);

		private:
			RPtr<Scene> m_activeScene;
			std::vector<RPtr<Scene>> m_scenes;
		};
	}
}