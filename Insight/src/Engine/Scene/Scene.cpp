#include "ispch.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Component/Component.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Event/EventManager.h"
#include "Engine/Event/ApplicationEvent.h"
#include "Engine/Instrumentor/Instrumentor.h"
#include "Engine/Graphics/RenderList.h"
#include "Engine/Core/Maths/Frustum.h"

#include "Engine/Module/WindowModule.h"

#include "Engine/Serialization/File/SerializableFile.h"

#include "glm/gtc/matrix_transform.hpp"
#include <ostream>

Scene* Scene::s_CurrentScene;

Scene::Scene(const std::string& sceneName)
	: m_sceneName(sceneName)
	, m_entityManager(EntityManager(this, m_componentManager))
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

Entity Scene::CreateEntity(const std::string& name)
{
	Entity& e = m_entityManager.CreateEntity();
	e.SetName(name);
	e.AddComponent<TransformComponent>();
	return e;
}

void Scene::RemoveEntity(const EntityID& entity)
{
	m_entityManager.DestroyEntity(entity);
	m_componentManager.EntityDestroyed(entity);
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

std::vector<Entity> Scene::GetEntites()
{
	return m_entityManager.GetAliveEntities();
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

}

void Scene::Serialize(const std::string& file)
{
	//IS_PROFILE_FUNCTION();

	//Serialization::SerializableFile* serializableFile = Serialization::SerializableFile::New();
	//auto root = serializableFile->GetNewElement("Scene");
	//for (auto it = m_registry.begin(); it != m_registry.end(); ++it)
	//{
	//	Serialization::SerializableElement* entityNode = root->AddChild("Entity");
	//	(*it)->Serialize(entityNode);
	//}

	//serializableFile->SaveFile(file);
	//::Delete(serializableFile);

	//EventManager::Dispatch<SerializeEvent>(EventType::Serialize, SerializeEvent());
}

void Scene::Deserialize(const std::string& file)
{/*
	{
		IS_PROFILE_SCOPE("[Scene::Deserialize] 'Deserialize' all scene objects");

		Serialization::SerializableFile* serializableFile = Serialization::SerializableFile::New();

		if (!serializableFile->LoadFile(file))
		{
			return;
		}

		Serialization::SerializableElement* sceneNode = serializableFile->GetFirstChild();
		Serialization::SerializableElement* entity = sceneNode->GetFirstChild();
		if (entity != nullptr)
		{
			do
			{
				auto typeElement = entity->GetFirstAttribute("Type");
				if (typeElement)
				{
					std::string type = typeElement->GetValue();
					Entity* s = Serialization::Serializable::NewInstanceFromType<Entity>(type);
					s->Deserialize(entity);
					m_registry.push_back(dynamic_cast<Entity*>(s));
				}
				entity = entity->NextSibling();
			} while (entity != nullptr);
		}
		::Delete(serializableFile);
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
	*/
}

void Scene::OnBeginPlay()
{
}

void Scene::OnEndPlay()
{
}

void Scene::OnUpdate(const float& deltaTime)
{
	IS_PROFILE_FUNCTION();

	if (!m_isPlaying)
	{
		return;
	}

	m_componentManager.Update(deltaTime);
}

void Scene::OnDraw(Insight::Graphics::RenderList* renderList)
{
	// Get the first camera only if the scene is playing.
	if (IsPlaying())
	{
		auto& cameraComponents = m_componentManager.GetAllComponents<CameraComponent>();
		CameraComponent& camera = cameraComponents.at(0);
		if (camera.IsValid())
		{
			renderList->CameraTransform = camera.GetViewMatrix();
			renderList->CameraProjection = glm::perspective(glm::radians(45.0f), (float)Window::GetWidth() / (float)Window::GetHeight(), 0.1f, 1000.0f);//camera.GetProjMatrix();
		}
	}

	Insight::Maths::Frustum cameraFrustum;
	cameraFrustum.Update(renderList->CameraProjection, renderList->CameraTransform);

	auto& meshComponents = m_componentManager.GetAllComponents<MeshComponent>();
	for (auto& com : meshComponents)
	{
		if (!com.IsValid())
		{
			return;
		}

		TransformComponent& transformComponent = com.GetEntity().GetComponent<TransformComponent>();
		if (cameraFrustum.CheckSphere(transformComponent.GetPostion(), 50.0f))
		{
			com.OnDraw(renderList, transformComponent.GetTransform(), cameraFrustum);
		}
	}
}

void Scene::Clean()
{
	IS_PROFILE_FUNCTION();

	if (!m_isPlaying)
	{
		return;
	}
}