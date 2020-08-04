#include <Insight.h>
#include <../vendor/glm/glm/gtx/string_cast.hpp>

using namespace Insight::Maths;
using namespace Insight;

class Sandbox : public Insight::Application
{
public:
	Entity* e = nullptr;
	std::vector<TransformComponent*> transformComponents;
	Model* testModel = nullptr;

	Sandbox() : Insight::Application()
	{ }

	virtual void Create() override
	{
		IS_PROFILE_FUNCTION();

		bool test = true;

		if (test)
		{
			Model* m = NEW_ON_HEAP(Model, "./models/Survival_BackPack_2/backpack.obj");
			//Model* m = Model::Create("./models/Test/TestCube.fbx");
			Model* n = NEW_ON_HEAP(Model, "./models/nano/nanosuit.fbx");

			for (size_t i = 0; i < 10; ++i)
			{
				//if (i % 2 == 0)
				{
					Entity* testModel = Entity::CreateFromModel(m);
					glm::vec3 pos = glm::vec3(rand() % 500, 0, rand() % 500);
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
	}

	virtual void Draw() override
	{

	}

	~Sandbox()
	{
		IS_PROFILE_FUNCTION();

		DELETE_ON_HEAP(testModel);
		e->Delete();
	}
};

Insight::Application* Insight::CreateApplication()
{
	return new Sandbox();
}