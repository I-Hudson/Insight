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
		if (Input::KeyUp(KEY_H))
		{
			IS_INFO("H was released.");
		}

		if (Input::KeyHeld(KEY_A))
		{
			IS_INFO("A is held.");
		}
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