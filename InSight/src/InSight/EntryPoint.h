#pragma once

#ifdef IS_PLATFORM_WINDOWS

extern Insight::Application* Insight::CreateApplication();
#ifdef IS_DEBUG
int main(int argc, char** argv)
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#endif
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
