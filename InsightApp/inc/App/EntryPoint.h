#pragma once
#include "Engine.h"
#include "Core/MemoryTracker.h"
#include "Core/Logger.h"
#include "Core/Memory.h"

#ifdef TESTING
#endif

extern Insight::App::Engine* CreateApplication();

int main(int argc, char** argv)
{
	{
		Insight::Core::MemoryTracker::Instance();
		Insight::Core::Logger::Init();

		Insight::App::Engine* app = CreateApplication();
		if (app->Init())
		{
			app->Update();
		}
		app->Destroy();
		DeleteTracked(app);

		//Insight::Core::MemoryTracker::Instance().Destroy();
	}
	return 0;
}

#ifdef TESTING
namespace test
{
	//TEST_CASE("EngineRunTest", "[vector]")
	//{
	//	Insight::Core::MemoryTracker::Instance();
	//	Insight::Core::Logger::Init();
	//
	//	Insight::App::Engine* app = nullptr;
	//	SECTION("Create engine")
	//	{
	//		app = CreateApplication();
	//		REQUIRE(app != nullptr);
	//	}
	//
	//	SECTION("Run engine for x frames")
	//	{
	//		REQUIRE(app->Init() == true);
	//		app->ForceClose();
	//		app->Update();
	//	}
	//
	//	SECTION("Destroy app")
	//	{
	//		app->Destroy();
	//		DeleteTracked(app);
	//	}
	//}
}
#endif