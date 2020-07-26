#pragma once

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef IS_PLATFORM_WINDOWS

extern Insight::Application* Insight::CreateApplication();

int main(int argc, char** argv)
{
	{
		Insight::Log::Init();

		auto app = Insight::CreateApplication();
		//app->Run();
		delete app;
	}

	return 0;
}

#endif // IS_PLATFORM_WINDOWS
