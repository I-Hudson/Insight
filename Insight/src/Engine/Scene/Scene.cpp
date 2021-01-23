#include "ispch.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Entitiy/Entity.h"
#include "Engine/Component/Component.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Event/EventManager.h"
#include "Engine/Event/ApplicationEvent.h"
#include "Engine/Instrumentor/Instrumentor.h"

#include "Engine/Serialization/File/SerializableFile.h"

#include <ostream>

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

	SharedPtr<Entity> Scene::CreateEntity(const std::string& name)
	{
		SharedPtr<Entity> e = Object::CreateObject<Entity>(name);
		e->AddComponent<TransformComponent>();
		m_registry.push_back(e);
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
			(*it).reset();
			m_registry.erase(it);
		}
	}

	void Scene::DeleteEntiy(SharedPtr<Entity> ptr)
	{
		auto it = std::find(m_registry.begin(), m_registry.end(), ptr);
		if (it != m_registry.end())
		{
			(*it).reset();

			m_registry.erase(it);
		}
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
		IS_CORE_ASSERT(s_CurrentScene != nullptr, "[Scene::ActiveScene] 's_CurrentScene' must be a valid pointer.");
		return s_CurrentScene;
	}

	void Scene::Load(const std::string& file)
	{
		Unload();
		Deserialize(file);
	}

	void Scene::Save(const std::string& file)
	{
		Serialize(file);
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

	void Scene::Serialize(const std::string& file)
	{
			IS_PROFILE_FUNCTION();

			UniquePtr<Serialization::SerializableFile> serializableFile = Serialization::SerializableFile::Create();
			auto root = serializableFile->GetNewElement("Scene");
			for (auto it = m_registry.begin(); it != m_registry.end(); ++it)
			{
				SharedPtr<Serialization::SerializableElement>entityNode = root->AddChild("Entity");
				(*it)->Serialize(entityNode);
			}

			serializableFile->SaveFile(file);
			serializableFile.reset();

			EventManager::Dispatch<SerializeEvent>(EventType::Serialize, SerializeEvent());
	}

	void Scene::Deserialize(const std::string& file)
	{
		{
			IS_PROFILE_SCOPE("[Scene::Deserialize] 'Deserialize' all scene objects");

			UniquePtr<Serialization::SerializableFile> serializableFile = Serialization::SerializableFile::Create();

			if (!serializableFile->LoadFile(file))
			{
				return;
			}

			SharedPtr<Serialization::SerializableElement> sceneNode = serializableFile->GetFirstChild();
			SharedPtr<Serialization::SerializableElement> entity = sceneNode->GetFirstChild();
			if (entity != nullptr)
			{
				do
				{
					auto typeElement = entity->GetFirstAttribute("Type");
					if (typeElement)
					{
						std::string type = typeElement->GetValue();
						SharedPtr<Entity> s = Serialization::Serializable::CreateInstanceFromType<Entity>(type);
						s->Deserialize(entity);
						m_registry.push_back(DynamicPointerCast<Entity>(s));
					}
					entity = entity->NextSibling().lock();
				} while (entity != nullptr);
			}

		}

		{
			IS_PROFILE_SCOPE("[Scene::Deserialize] 'OnCreate' all scene components");
			for (auto itE = m_registry.begin(); itE != m_registry.end(); ++itE)
			{
				auto components = (*itE)->GetAllComponents();
				for (auto itC = components.begin(); itC != components.end(); ++itC)
				{
					(*itC)->OnCreate();
				}
			}
		}

		EventManager::Dispatch<DeserializeEvent>(EventType::Deserialize, DeserializeEvent());
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