#pragma once
#include "Engine.h"
#include "Core/MemoryTracker.h"
#include "Core/Logger.h"
#include "Core/Memory.h"

#include "doctest.h"

extern Insight::App::Engine* CreateApplication();

int main(int argc, char** argv)
{
	Insight::Core::MemoryTracker::Instance();
	Insight::Core::Logger::Init();

#ifdef TESTING
	const char* args[] = { "-d", "--order-by=suite"};
	return doctest::Context(ARRAYSIZE(args), args).run();
#else
	Insight::App::Engine* app = CreateApplication();
	if (app->Init())
	{
		app->Update();
	}
	app->Destroy();
	DeleteTracked(app);

	//Insight::Core::MemoryTracker::Instance().Destroy();
	return 0;
#endif
}