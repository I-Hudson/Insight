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

		bool test = true;

		Model* m = Library::ModelLibrary::GetInstance()->GetAssetFromPath("./models/Survival_BackPack_2/backpack.obj");
		//Model* m = Model::Create("./models/Test/TestCube.fbx");
		//Model* n = Library::ModelLibrary::GetInstance()->GetAssetFromPath("./models/nano/nanosuit.fbx");

		if (test)
		{
			for (size_t i = 0; i < 1; ++i)
			{
				//if (i % 2 == 0)
				{
					Entity* testModel = Entity::CreateFromModel(m);
 					testModel->AddComponent<PlayerController>();
					glm::vec3 pos = glm::vec3(rand() % 50, 0, rand() % 50);
					testModel->GetComponent<TransformComponent>()->SetPosition(pos);
					transformComponents.push_back(testModel->GetComponent<TransformComponent>());
				}
				//else
				//{
				//	Entity* testModel = Entity::CreateFromModel(n);
				//	glm::vec3 pos;
				//	pos.x = -10 * i;
				//	pos.y = 0;
				//	pos.z = -10 * i;
				//	testModel->GetComponent<TransformComponent>()->SetPosition(pos);
				//}
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
			Scene::ActiveScene()->Load("sandbox.json");
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