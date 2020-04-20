#include <Insight.h>

using namespace Insight::Maths;

class Sandbox : public Insight::Application
{
public:
	Sandbox()
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