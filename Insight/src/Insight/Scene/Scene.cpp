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

	const std::string& Scene::GetSceneName() const
	{
		return m_sceneName;
	}

	void Scene::SetSceneName(const std::string& sceneName)
	{
		m_sceneName = sceneName;
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
		m_updateComponents.clear();
	}

	void Scene::Serialize()
	{
		{
			IS_PROFILE_SCOPE("XML Parse");
			using namespace tinyxml2;
			tinyxml2::XMLDocument doc;
			XMLNode* root = doc.NewElement("Scene");
			doc.InsertFirstChild(root);

			for (auto it = m_registry.begin(); it != m_registry.end(); ++it)
			{
				XMLNode* entityNode = doc.NewElement("Entity");
				(*it)->Serialize(entityNode, &doc);
				if (!entityNode->NoChildren())
				{
					root->InsertEndChild(entityNode);
				}
			}

			if (XMLError err = doc.SaveFile((DEFAULT_SAVE_PATH + m_sceneName + ".xml").c_str()))
			{
				IS_CORE_ASSERT(false, doc.ErrorIDToName(err));
			}

			Insight::EventManager::Dispatch<SerializeEvent>(EventType::Serialize, SerializeEvent());
		}
	}

	void Scene::Deserialize(const std::string& file)
	{
		{
			IS_PROFILE_SCOPE("[Scene::Deserialize] 'Deserialize' all scene objects");

			std::string filePath = file;
			if (filePath.find_last_of('.') == SIZE_MAX)
			{
				filePath.append(".xml");
			}

			tinyxml2::XMLDocument doc;
			tinyxml2::XMLError eResult = doc.LoadFile((DEFAULT_SAVE_PATH + filePath).c_str());
			if (eResult == tinyxml2::XML_SUCCESS)
			{
				tinyxml2::XMLNode* root = doc.FirstChild();
				tinyxml2::XMLNode* c = root->FirstChild();
				if (c != nullptr)
				{
					do
					{
						std::string type = c->FirstChildElement("Type")->ToElement()->GetText();
						Serialization::Serializable* s = Serialization::Serializable::CreateInstanceFromType<Serialization::Serializable>(type);
						s->Deserialize(c);
						c = c->NextSibling();
					} while (c != nullptr);
				}
			}
			else
			{
				IS_CORE_INFO("{0}", doc.ErrorIDToName(doc.ErrorID()));
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

		Insight::EventManager::Dispatch<DeserializeEvent>(EventType::Deserialize, DeserializeEvent());
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