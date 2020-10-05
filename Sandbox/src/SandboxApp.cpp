#include <Insight.h>

#include "PlayerController.h"

using namespace Insight;

class Sandbox : public Insight::Application
{
public:
	std::vector<TransformComponent*> transformComponents;
	Scene* m_sandboxScene;


	Sandbox() : Insight::Application()
	{ }

	virtual void Create() override
	{
		IS_PROFILE_FUNCTION();

		m_sandboxScene = NEW_ON_HEAP(Scene, "Sandbox");
		m_sandboxScene->SetActiveScene();

		Entity* mainCamera = Entity::Create("MainCamera");
		mainCamera->AddComponent<CameraComponent>();

		bool test = false;

		Library::ModelLibrary::GetInstance()->LoadAssetsFromFolder("./models", true);
		Model* m = Library::ModelLibrary::GetInstance()->GetAssetFromPath("./models/Survival_BackPack_2/backpack.obj");

		Entity::Create("Root Entity")->AddChild("child_1");

		if (test)
		{
			for (size_t i = 0; i < 5; ++i)
			{
				{
					Entity* testModel = Entity::CreateFromModel(m);
 					testModel->AddComponent<PlayerController>();
					glm::vec3 pos = glm::vec3(/*rand() % 50*/0, 0, /*rand() % 50*/0);
					testModel->GetComponent<TransformComponent>()->SetPosition(pos);
					transformComponents.push_back(testModel->GetComponent<TransformComponent>());
				}
			}
		}
	}

	virtual void Update(const float deltaTime) override
	{
		IS_PROFILE_FUNCTION();

		if (Input::KeyDown(KEY_LEFT_CONTROL) && Input::KeyDown(KEY_S))
		{
			Scene::ActiveScene()->Save();
		}

		if (Input::KeyDown(KEY_LEFT_CONTROL) && Input::KeyDown(KEY_F))
		{
			Scene::ActiveScene()->Load("Sandbox.xml");
		}

		Scene::ActiveScene()->OnUpdate(deltaTime);
	}

	virtual void Draw() override
	{

	}

	virtual void OnFrameEnd() override
	{
		Scene::ActiveScene()->Clean();
	}

	~Sandbox()
	{
		IS_PROFILE_FUNCTION();

		DELETE_ON_HEAP(m_sandboxScene);
	}
};

Insight::Application* Insight::CreateApplication()
{
	return new Sandbox();
}