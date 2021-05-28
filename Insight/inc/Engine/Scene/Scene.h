#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Entity/EntityManager.h"
#include "Engine/Entity/ComponentManager.h"
#include <string>

const std::string DEFAULT_SAVE_PATH = "data/scenes/";

namespace Insight::Graphics
{
	struct RenderList;
}

class IS_API Scene
{
public:
	Scene(const std::string& sceneName = "Default");
	~Scene();

	Entity CreateEntity(const std::string& name = std::string());
	void RemoveEntity(const EntityID& entity);

	const std::string& GetSceneName() const;
	void SetSceneName(const std::string& sceneName);

	void SetActiveScene();
	static Scene* ActiveScene();

	std::vector<Entity> GetEntites();

	void Save(const std::string& file);
	void Load(const std::string& file);
	void Unload();

	void Serialize(const std::string& file);
	void Deserialize(const std::string& file);

	void OnBeginPlay();
	void OnEndPlay();

	void OnUpdate(const float& deltaTime);
	void OnDraw(Insight::Graphics::RenderList* renderList);
	void Clean();

	bool IsPlaying() const { return m_isPlaying; }

	Entity GetEntity(const EntityID& entity) { return m_entityManager.GetEntity(entity); }
	template<typename T>
	std::array<T, MAX_ENTITIES_COUNT>& GetAllComponents()
	{
		return m_componentManager.GetAllComponents<T>();
	}

private:
	EntityManager& GetEntityManager() { return m_entityManager; }
	ComponentManager& GetComponentManager() { return m_componentManager; }

private:
	EntityManager m_entityManager;
	ComponentManager m_componentManager;

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	std::string m_sceneName;

	bool m_isPlaying;

	static Scene* s_CurrentScene;

	friend EntityManager;
	friend ComponentManager;
	friend class Entity;
};