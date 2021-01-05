#pragma once

#ifdef IS_PLATFORM_WINDOWS

extern Insight::Application* Insight::CreateApplication();
#if defined(IS_DEBUG)
int main(int argc, char** argv)
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#endif
{
	{
		Insight::Log::Init();
		IS_PROFILE_BEGIN_SESSION();
		//::Insight::Profile::Instrumentor::Get().BeginSession("App Begin", "profileResults.json");
		auto app = Insight::CreateApplication();
		app->Run();
		delete app;
		IS_PROFILE_END_SESSION();
		IS_PROFILE_SAVE_SESSION("AppProfile.json");
	}

	return 0;
}
#endif // IS_PLATFORM_WINDOWS
