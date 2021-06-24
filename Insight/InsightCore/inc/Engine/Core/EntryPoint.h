#pragma once

#ifdef IS_PLATFORM_WINDOWS

extern Insight::Application* CreateApplication();
#if defined(IS_DEBUG)
int main(int argc, char** argv)
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#endif
{
	{
		Log::Init();
		IS_PROFILE_BEGIN_SESSION("AppProfile.json");
		//::Profile::Instrumentor::Get().BeginSession("App Begin", "profileResults.json");
		auto app = CreateApplication();
		app->Run();
		delete app;
		IS_PROFILE_END_SESSION();
		IS_PROFILE_SAVE_SESSION();
	}

	return 0;
}
#endif // IS_PLATFORM_WINDOWS
