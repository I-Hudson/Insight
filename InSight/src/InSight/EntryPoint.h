#pragma once

#ifdef IS_PLATFORM_WINDOWS

extern Insight::Application* Insight::CreateApplication();

int main(int argc, char** argv)
{
	{
		Insight::Log::Init();
		Insight::Profile::Instrumentor::Get().BeginSession("App Begin");
		auto app = Insight::CreateApplication();
		app->Run();
		delete app;
		Insight::Profile::Instrumentor::Get().EndSession();
	}

	return 0;
}

#endif // IS_PLATFORM_WINDOWS
