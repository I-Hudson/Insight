#include <Insight.h>
#include <../vendor/glm/glm/gtx/string_cast.hpp>

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
			for (size_t i = 0; i < 50; ++i)
			{
				//if (i % 2 == 0)
				{
					Entity* testModel = Entity::CreateFromModel(m);
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

		double x, y;
		Input::GetMousePosition(&x, &y);

		for (size_t i = 0; i < transformComponents.size(); ++i)
		{
			TransformComponent* tc = transformComponents[i];
			if (Input::KeyHeld(KEY_UP))
			{
				tc->SetPosition(tc->GetPostion() + glm::vec3(0, 0, 1) * Time::GetDeltaTime());
			}
			if (Input::KeyHeld(KEY_DOWN))
			{
				tc->SetPosition(tc->GetPostion() + glm::vec3(0, 0, -1) * Time::GetDeltaTime());
			}
			if (Input::KeyHeld(KEY_LEFT))
			{
				tc->SetPosition(tc->GetPostion() + glm::vec3(-1, 0, 1) * Time::GetDeltaTime());
			}
			if (Input::KeyHeld(KEY_RIGHT))
			{
				tc->SetPosition(tc->GetPostion() + glm::vec3(1, 0, 1) * Time::GetDeltaTime());
			}
		}

		if (Input::KeyDown(KEY_LEFT_CONTROL) && Input::KeyDown(KEY_S))
		{
			Scene::ActiveScene()->Serialize();
		}

		if (Input::KeyDown(KEY_LEFT_CONTROL) && Input::KeyDown(KEY_F))
		{
			Scene::ActiveScene()->Deserialize("sandbox.json");
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