#include "Scene/SceneManager.h"

#include "Core/Profiler.h"
#include "Core/Logger.h"

namespace Insight
{
	namespace App
	{
		Scene::Scene()
			: m_sceneName("Default")
		{
#ifdef ECS_ENABLED
			m_ecsWorld = MakeUPtr<ECS::ECSWorld>();
#else
			m_entityManager.SetScene(this);
#endif
		}

		Scene::Scene(std::string sceneName)
			: m_sceneName(std::move(sceneName))
		{
#ifdef ECS_ENABLED
			m_ecsWorld = MakeUPtr<ECS::ECSWorld>();
#else
			m_entityManager.SetScene(this);
#endif
		}

		Scene::~Scene()
		{

		}

		void Scene::Destroy()
		{
#ifdef ECS_ENABLED
			m_ecsWorld->Destroy();
#else
#endif
		}

		void Scene::EarlyUpdate()
		{
			IS_PROFILE_FUNCTION();
#ifdef ECS_ENABLED
			m_ecsWorld->EarlyUpdate();
#else
			m_entityManager.EarlyUpdate();
#endif
		}

		void Scene::Update(const float deltaTime)
		{
			IS_PROFILE_FUNCTION();
#ifdef ECS_ENABLED
			m_ecsWorld->Update(deltaTime);
#else
			m_entityManager.Update(deltaTime);
#endif
		}

		void Scene::LateUpdate()
		{
			IS_PROFILE_FUNCTION();
#ifdef ECS_ENABLED
			m_ecsWorld->LateUpdate();
#else
			m_entityManager.LateUpdate();
#endif
		}

		void Scene::SetOnlySearchable(bool onlySearchable)
		{
			m_onlySearchable = onlySearchable;
		}

		Ptr<ECS::Entity> Scene::AddEntity()
		{
			return AddEntity("");
		}

		Ptr<ECS::Entity> Scene::AddEntity(std::string entity_name)
		{
#ifdef ECS_ENABLED
			return m_ecsWorld->AddEntity(entity_name);
#else
			Ptr<ECS::Entity> e = m_entityManager.AddNewEntity(entity_name);
			m_root_entities.push_back(e);
			return e;
#endif
		}

		Ptr<ECS::Entity> Scene::GetEntityByName(std::string entity_name) const
		{
#ifdef ECS_ENABLED
			return m_ecsWorld->GetEntityByName(entity_name);
#else
			return m_entityManager.GetEntityByName(entity_name);
#endif
		}

		void Scene::RemoveEntity(Ptr<ECS::Entity>& entity)
		{
			ECS::Entity* e = entity.Get();
#ifdef ECS_ENABLED
			m_ecsWorld->RemoveEntity(e);
#else
			IS_UNUSED(std::remove_if(m_root_entities.begin(), m_root_entities.end(), [e](const Ptr<ECS::Entity>& other_entity)
				{
					return e == other_entity.Get();
				}));
			m_entityManager.RemoveEntity(e);
#endif
		}

		std::vector<Ptr<ECS::Entity>> Scene::GetAllEntitiesWithComponentByName(std::string_view component_type) const
		{
#ifdef ECS_ENABLED
			return m_ecsWorld->GetAllEntitiesWithComponentByName(component_type);
#else
			return m_entityManager.GetAllEntitiesWithComponentByName(component_type);
#endif
		}

		std::vector<Ptr<ECS::Entity>> Scene::GetAllEntities() const
		{
			return m_root_entities;
		}


		SceneManager::SceneManager()
		{
		}

		SceneManager::~SceneManager()
		{
			Destroy();
		}

		void SceneManager::EarlyUpdate()
		{
			IS_PROFILE_FUNCTION();
			for (size_t sceneIdx = 0; sceneIdx < m_activeScenes.size(); ++sceneIdx)
			{
				if (m_activeScenes.at(sceneIdx).IsValid())
				{
					m_activeScenes.at(sceneIdx)->EarlyUpdate();
				}
			}
		}

		void SceneManager::Update(const float deltaTime)
		{
			IS_PROFILE_FUNCTION();
			for (size_t sceneIdx = 0; sceneIdx < m_activeScenes.size(); ++sceneIdx)
			{
				if (m_activeScenes.at(sceneIdx).IsValid())
				{
					m_activeScenes.at(sceneIdx)->Update(deltaTime);
				}
			}
		}

		void SceneManager::LateUpdate()
		{
			IS_PROFILE_FUNCTION();
			for (size_t sceneIdx = 0; sceneIdx < m_activeScenes.size(); ++sceneIdx)
			{
				if (m_activeScenes.at(sceneIdx).IsValid())
				{
					m_activeScenes.at(sceneIdx)->LateUpdate();
				}
			}
		}

		void SceneManager::Destroy()
		{
			for (RPtr<Scene> const& ptr : m_scenes)
			{
				ptr->Destroy();
			}
			m_scenes.clear();
		}

		WPtr<Scene> SceneManager::CreateScene(std::string sceneName)
		{
			RPtr<Scene> scene = MakeRPtr<Scene>(std::move(sceneName));
			m_scenes.push_back(scene);
			return m_scenes.back();
		}

		WPtr<Scene> SceneManager::CreatePersistentScene(std::string sceneName)
		{
			WPtr<Scene> scene = CreateScene(sceneName);
			scene.Lock()->m_persistentScene = true;
			AddActiveScene(scene);
			return scene;
		}

		void SceneManager::SetActiveScene(WPtr<Scene> scene)
		{
			std::vector<RPtr<Scene>> scenesToRemove;
			for (size_t sceneIdx = 0; sceneIdx < m_activeScenes.size(); ++sceneIdx)
			{
				if (m_activeScenes.at(sceneIdx).IsValid() && !m_activeScenes.at(sceneIdx)->IsPersistentScene())
				{
					scenesToRemove.push_back(m_activeScenes.at(sceneIdx));
				}
			}

			// Remove all non persistent scenes
			for (size_t i = 0; i < scenesToRemove.size(); ++i)
			{
				RemoveScene(scenesToRemove.at(i));
			}

			m_activeScenes.push_back(scene.Lock());
		}

		void SceneManager::AddActiveScene(WPtr<Scene> scene)
		{
			if (std::find_if(m_activeScenes.begin(), m_activeScenes.end(), [&scene](const RPtr<Scene>& activeScene)
				{
					return activeScene == scene.Lock();
				}) == m_activeScenes.end())
			{
				m_activeScenes.push_back(scene.Lock());
			}
		}

		void SceneManager::RemoveScene(WPtr<Scene> scene)
		{
			int index = 0;
			for (RPtr<Scene> const& ptr : m_scenes)
			{
				if (scene == ptr)
				{
					scene.Reset();
					break;
				}
				++index;
			}

			RPtr<Scene> toRemoveScene = std::move(m_scenes.at(index));
			m_scenes.erase(m_scenes.begin() + index);

			auto activeSceneItr = std::find_if(m_activeScenes.begin(), m_activeScenes.end(), [&toRemoveScene](const RPtr<Scene>& scene)
				{
					return toRemoveScene == scene;
				});

			if (activeSceneItr != m_activeScenes.end())
			{
				activeSceneItr->Reset();
				m_activeScenes.erase(activeSceneItr);
			}
			toRemoveScene->Destroy();
		}

		WPtr<Scene> SceneManager::GetActiveScene() const
		{
			for (size_t sceneIdx = 0; sceneIdx < m_activeScenes.size(); ++sceneIdx)
			{
				if (m_activeScenes.at(sceneIdx).IsValid() && !m_activeScenes.at(sceneIdx)->IsOnlySearchable())
				{
					return m_activeScenes.at(sceneIdx);
				}
			}
			IS_CORE_ERROR("[SceneManager::GetActiveScene] No active scene found.");
			return WPtr<Scene>();
		}

		WPtr<Scene> SceneManager::FindSceneByName(std::string_view sceneName)
		{
			for (RPtr<Scene> const& scene : m_scenes)
			{
				if (scene->GetSceneName() == sceneName)
				{
					return scene;
				}
			}
			return WPtr<Scene>();
		}
	
		WPtr<Scene> SceneManager::GetSceneFromIndex(u32 index) const
		{
			if (index < m_activeScenes.size())
			{
				return m_activeScenes.at(index);
			}
			return WPtr<Scene>();
		}

		std::vector<WPtr<Scene>> SceneManager::GetAllScenes() const
		{
			std::vector<WPtr<Scene>> scenes;
			for (size_t i = 0; i < m_activeScenes.size(); ++i)
			{
				scenes.push_back(m_activeScenes.at(i));
			}
			return scenes;
		}
	}
}

#ifdef TESTING
#include "doctest.h"
#include "Core/Memory.h"
		TEST_SUITE("SceneManagerSingleton")
		{
			using namespace Insight::App;

			UPtr<SceneManager> sceneManager;
			TEST_CASE("Make SceneManger UPtr")
			{
				sceneManager = MakeUPtr<SceneManager>();
				CHECK(sceneManager.IsValid());
			}

			TEST_CASE("SceneManager Singleton valid")
			{
				CHECK(sceneManager->IsValidInstance());
			}
		}
#endif