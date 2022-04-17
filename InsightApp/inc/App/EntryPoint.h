#pragma once
#include "Engine.h"
#include "Core/MemoryTracker.h"
#include "Core/Logger.h"
#include "Core/Memory.h"

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