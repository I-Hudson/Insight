#include <Insight.h>
#include <../vendor/glm/glm/gtx/string_cast.hpp>

using namespace Insight::Maths;

class Sandbox : public Insight::Application
{
public:
	Entity* e = nullptr;
	TransformComponent* tc = nullptr;
	Model* testModel = nullptr;

	Sandbox() : Insight::Application()
	{ }

	virtual void Create() override
	{
		Model* m = Model::Create("./models/Survival_BackPack_2/backpack.obj");
		Entity* testModel = Entity::CreateFromModel(m);

		int* i1 = new int;
		*i1 = 1;
		int* i2 = i1;

		delete i1;
		IS_INFO(*i2);
	}

	virtual void Update(const float deltaTime) override
	{
		double x, y;
		Input::GetMousePosition(&x, &y);

		if (Input::KeyHeld(KEY_UP))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(0, 0, 1));
		}
		if (Input::KeyHeld(KEY_DOWN))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(0, 0, -1));
		}
		if (Input::KeyHeld(KEY_LEFT))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(-1, 0, 1));
		}
		if (Input::KeyHeld(KEY_RIGHT))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(1, 0, 1));
		}
	}

	virtual void Draw() override
	{

	}

	~Sandbox()
	{
		DELETE_ON_HEAP(testModel);
		e->Delete();
	}
};

Insight::Application* Insight::CreateApplication()
{
	return new Sandbox();
}