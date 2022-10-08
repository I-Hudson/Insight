#pragma once
#include "Engine.h"
#include "Core/MemoryTracker.h"
#include "Core/Logger.h"
#include "Core/Memory.h"

#ifdef TEST_ENABLED
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include "doctest.h"
#endif 

extern Insight::App::Engine* CreateApplication();

int main(int argc, char** argv)
{
	Insight::Core::MemoryTracker::Instance();
	Insight::Core::Logger::Init();

#ifdef TEST_ENABLED
	const char* args[] = { "-d", "--order-by=suite"};
	return doctest::Context(ARRAYSIZE(args), args).run();
#else
	Insight::App::Engine* app = CreateApplication();
	if (app->Init(argc, argv))
	{
		app->Update();
	}
	app->Destroy();
	DeleteTracked(app);

	///Insight::Core::MemoryTracker::Instance().Destroy();
	return 0;
#endif
}