#pragma once
#include "Engine.h"

extern Insight::Core::Engine* CreateApplication();

int main(int argc, char** argv)
{
	Insight::Core::Engine* app = CreateApplication();
	app->Init();
	app->Update();
	app->Destroy();
	delete app;
	return 0;
}