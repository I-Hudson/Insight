#pragma once

#include "Engine/Core/Core.h"
#include <string>

class Entity;
class Component;

const std::string DEFAULT_SAVE_PATH = "data/scenes/";

	class IS_API Scene : std::enable_shared_from_this<Scene>
	{
	public:
		Scene(const std::string& sceneName = "Default");
		~Scene();

		Entity* CreateEntity(const std::string& name = std::string());
		void DeleteEntiy(const std::string uuid);
		void DeleteEntiy(Entity* ptr);

		const std::string& GetSceneName() const;
		void SetSceneName(const std::string& sceneName);

		void SetActiveScene();
		static Scene* ActiveScene();
		const std::vector<Entity*> GetEntites() { return m_registry; }

		void Save(const std::string& file);
		void Load(const std::string& file);
		void Unload();

		void Serialize(const std::string& file);
		void Deserialize(const std::string& file);

		void OnUpdate(const float& deltaTime);
		void OnViewportResize(uint32_t width, uint32_t height);
		void Clean();

		template<typename T>
		T* FindFirstComponent();

	private:
		std::vector<Entity*> m_registry;
		std::vector<Component*> m_updateComponents;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		std::string m_sceneName;

		bool m_isPlaying;

		static Scene* s_CurrentScene;

		friend class Entity;
	};

	template<typename T>
	inline T* Scene::FindFirstComponent()
	{
		for (auto it = m_registry.begin(); it != m_registry.end(); ++it) 
		{
			if ((*it)->HasComponent<T>())
			{
				return (*it)->GetComponent<T>();
			}
		}

		return nullptr;
	}