#include "ispch.h"

#include "Insight/Scene/Scene.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/Component/Component.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Component/TransformComponent.h"
#include "Insight/Event/EventManager.h"
#include "Insight/Event/ApplicationEvent.h"
#include "Insight/Instrumentor/Instrumentor.h"

#if defined(IS_EDITOR) && defined(IMGUI_ENABLED)
#include "imgui.h"
#endif

#include <ostream>

namespace Insight
{
	Scene* Scene::s_CurrentScene;

	Scene::Scene(const std::string& sceneName)
		: m_sceneName(sceneName)
#ifdef IS_EDITOR
		, m_isPlaying(false)
#else 
		, m_isPlaying(true)
#endif
	{ }

	Scene::~Scene()
	{
		Unload();
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

	void Scene::Load(const std::string& file)
	{
		Unload();
		Deserialize(file);
	}

	void Scene::Save()
	{
		Serialize();
	}

	void Scene::Unload()
	{
		while (m_registry.size() > 0)
		{
			DeleteEntiy(*m_registry.begin());
		}
		m_registry.clear();
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
			{
				IS_PROFILE_SCOPE("[Scene::Deserialize] 'Deserialize' all scene objects");
				for (auto it = objects.begin(); it != objects.end(); ++it)
				{
					std::string type = (*it)["Type"];
					Serialization::Serializable* s = Serialization::Serializable::CreateInstanceFromType<Serialization::Serializable>(type);
					s->Deserialize(*it);
				}
			}

			{
				IS_PROFILE_SCOPE("[Scene::Deserialize] 'OnCreate' all scene components");
				for (auto itE = m_registry.begin(); itE != m_registry.end(); ++itE)
				{
					auto components = (*itE)->GetAllComponents();
					for (auto itC = components->begin(); itC != components->end(); ++itC)
					{
						(*itC)->OnCreate();
					}
				}
			}

			in.close();
			EventManager::Dispatch(EventType::Deserialize, DeserializeEvent());
		}
	}

	void Scene::OnUpdate(const float& deltaTime)
	{
		IS_PROFILE_FUNCTION();

#if defined(IS_EDITOR) && defined(IMGUI_ENABLED)
		ImGui::Begin("Scene");
		ImGui::Checkbox("Scene playing", &m_isPlaying);
		ImGui::End();
#endif

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