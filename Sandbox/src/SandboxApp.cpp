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
		testModel = NEW_ON_HEAP(Model, "./models/Survival_BackPack_2/backpack.obj");
		for (size_t i = 0; i < 20; ++i)
		{
			Entity::CreateFromModel(testModel);
		}
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