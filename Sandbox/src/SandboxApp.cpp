#include <Insight.h>

using namespace Insight::Maths;

class Sandbox : public Insight::Application
{
public:
	Sandbox()
	{

	}

	virtual void Update(const float deltaTime) override
	{
		double x, y;
		Input::GetMousePosition(&x, &y);

		IS_CORE_INFO("MouseX: {0}, MouseY: {1}", x, y);
	}

	virtual void Draw() override
	{

	}

	~Sandbox()
	{

	}
};

Insight::Application* Insight::CreateApplication()
{
	return new Sandbox();
}