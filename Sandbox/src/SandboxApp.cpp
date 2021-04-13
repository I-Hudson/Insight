#include <Insight.h>
#include "PlayerController.h"

class Sandbox : public Application
{
public:
	std::vector<TransformComponent*> transformComponents;
	Scene* m_sandboxScene;


	Sandbox() : Application()
	{ }

	virtual void Create() override
	{
		IS_PROFILE_FUNCTION();

		m_sandboxScene = new Scene("Sandbox");
		m_sandboxScene->SetActiveScene();

		Entity* mainCamera = Entity::New("MainCamera");
		mainCamera->AddComponent<CameraComponent>();
		mainCamera->AddComponent<PlayerController>();

		bool test = false;

		//Entity::Create("Root Entity")->AddChild("child_1");

		Library::ModelLibrary::Instance()->LoadAssetsFromFolder("./data/models", true);
		if (test)
		{
			Model* m = Library::ModelLibrary::Instance()->GetAssetFromPath("./models/Test/testCube.fbx"); //". / models / Survival_BackPack_2 / backpack.obj");
			for (size_t i = 0; i < 50; ++i)
			{
				{
					Entity* testModel = Entity::CreateFromModel(m);
 					testModel->AddComponent<PlayerController>();
					glm::vec3 pos = glm::vec3(rand() % 50, 0, rand() % 50);
					testModel->GetComponent<TransformComponent>()->SetPosition(pos);
					transformComponents.push_back(testModel->GetComponent<TransformComponent>());
				}
			}
		}
	}

	virtual void Update(const float deltaTime) override
	{
		IS_PROFILE_FUNCTION();

		if (Input::IsKeyDown(KEY_LEFT_CONTROL) && Input::IsKeyDown(KEY_S))
		{
			Scene::ActiveScene()->Save("");
		}

		if (Input::IsKeyDown(KEY_LEFT_CONTROL) && Input::IsKeyDown(KEY_F))
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

		delete m_sandboxScene;
	}
};

Application* CreateApplication()
{
	return new Sandbox();
}