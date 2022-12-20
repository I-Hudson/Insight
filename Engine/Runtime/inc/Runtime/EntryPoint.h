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

#ifdef IS_DX12_ENABLED
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 706; } // https://devblogs.microsoft.com/directx/directx12agility/
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\"; } // Path to .dll is root of the .exe. 
#endif

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