#pragma once

#include "Insight/Core.h"
#include "Insight/Serialization/Serializable.h"

class Entity;

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

		void Serialize();
		void Deserialize(const std::string& file);

		void OnUpdate(float deltaTime);
		void OnViewportResize(uint32_t width, uint32_t height);

	private:
		std::vector<Entity*> m_registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		std::string m_sceneName;

		static Scene* s_CurrentScene;

		friend class Entity;
	};
}