#pragma once

#include "Insight/Core.h"
#include "Insight/Serialization/Serializable.h"

class Entity;
class Component;

namespace Insight
{
	class Scene
	{
	public:
		Scene(const std::string& sceneName = "Default");
		~Scene();

		Entity* CreateEntity(const std::string& name = std::string());
		void DeleteEntiy(const std::string uuid);
		void DeleteEntiy(Entity* ptr);

		void SetActiveScene();
		static Scene* ActiveScene();

		void Load(const std::string& file);
		void Save();
		void Unload();

		void Serialize();
		void Deserialize(const std::string& file);

		void OnUpdate(const float& deltaTime);
		void OnViewportResize(uint32_t width, uint32_t height);
		void Clean();

	private:
		std::vector<Entity*> m_registry;
		std::vector<Component*> m_updateComponents;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		std::string m_sceneName;

		bool m_isPlaying;

		static Scene* s_CurrentScene;

		friend class Entity;
	};
}