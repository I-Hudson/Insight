#include "ispch.h"

#include "Insight/Scene/Scene.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/Component/Component.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Component/TransformComponent.h"
#include "Insight/Event/EventManager.h"
#include "Insight/Event/ApplicationEvent.h"
#include "Insight/Instrumentor/Instrumentor.h"

#include <ppltasks.h>
#include <ostream>

namespace Insight
{
	Scene* Scene::s_CurrentScene;

	Scene::Scene(const std::string& sceneName)
		: m_sceneName(sceneName)
		, m_isPlaying(false)
	{ }

	Scene::~Scene()
	{
		while (m_registry.size() > 0)
		{
			DeleteEntiy(*m_registry.begin());
		}
		m_registry.clear();
	}

	Entity* Scene::CreateEntity(const std::string& name)
	{
		Entity* e = NEW_ON_HEAP(Entity, name);
		e->AddComponent<TransformComponent>();

		return e;
	}

	void Scene::DeleteEntiy(const std::string uuid)
	{
		bool found = false;
		auto it = m_registry.begin();
		for (; it != m_registry.end(); ++it)
		{
			if ((*it)->GetUUID() == uuid)
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			DELETE_ON_HEAP((*it));
			m_registry.erase(it);
		}
	}

	void Scene::DeleteEntiy(Entity* ptr)
	{
		auto it = std::find(m_registry.begin(), m_registry.end(), ptr);
		DELETE_ON_HEAP((*it));

		m_registry.erase(it);
	}

	void Scene::SetActiveScene()
	{
		s_CurrentScene = this;
	}

	Scene* Scene::ActiveScene()
	{
		return s_CurrentScene;
	}

	void Scene::Serialize()
	{
		std::ofstream out;
		std::string fileName(m_sceneName);
		fileName += ".json";
		out.open(fileName);
		if (out.is_open())
		{			
			std::vector<json> objects;
			for (auto it = m_registry.begin(); it != m_registry.end(); ++it)
			{
				json data;
				(*it)->Serialize(data);
				if (!data.empty())
				{
					objects.push_back(data);
				}
			}

			json data;
			data["SceneName"] = m_sceneName;
			for (size_t i = 0; i < objects.size(); ++i)
			{
				data["Objects"][objects[i]["UUID"].get<std::string>()] = objects[i];
			}
			out << data.dump(4);
			out.close();
		}
	}

	void Scene::Deserialize(const std::string& file)
	{
		std::ifstream in;
		in.open(file);
		if (in.is_open())
		{
			json data;
			in >> data;
	
			m_sceneName = data["SceneName"];
			json objects = data["Objects"];
			for (auto it = objects.begin(); it != objects.end(); ++it)
			{
				std::string type = (*it)["Type"];
				Serialization::Serializable* s = Serialization::SerializableRegistry::GetTypes().find(type)->second();
				s->Deserialize(*it);
			}
			in.close();
			EventManager::Dispatch(EventType::Deserialize, DeserializeEvent());
		}
	}

	void Scene::OnUpdate(const float& deltaTime)
	{
		IS_PROFILE_FUNCTION();

		if (!m_isPlaying)
		{
			return;
		}

		for (auto it = m_updateComponents.begin(); it != m_updateComponents.end(); ++it)
		{
			(*it)->OnUpdate(deltaTime);
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{

	}

	void Scene::Clean()
	{
		IS_PROFILE_FUNCTION();

		if (!m_isPlaying)
		{
			return;
		}

		for (auto it = m_updateComponents.begin(); it != m_updateComponents.end(); ++it)
		{
			(*it)->Clean();
		}
	}
}