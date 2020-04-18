#pragma once


#ifdef IS_PLATFORM_WINDOWS

extern Insight::Application* Insight::CreateApplication();

int main(int argc, char** argv)
{
	//Insight::Log::Init();
	//IS_CORE_WARN("Logger Inialised!");
	//Insight::Log::GetClientLogger()->info("Hello!");

	auto app = Insight::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#endif // IS_PLATFORM_WINDOWS
