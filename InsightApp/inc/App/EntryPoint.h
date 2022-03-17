#pragma once
#include "Engine.h"
#include "Core/MemoryTracker.h"

extern Insight::App::Engine* CreateApplication();

int main(int argc, char** argv)
{
	Insight::Core::MemoryTracker::Instance();
	Insight::App::Engine* app = CreateApplication();
	if (app->Init())
	{
		app->Update();
	}
	app->Destroy();
	delete app;

	//Insight::Core::MemoryTracker::Instance().Destroy();

	return 0;
}