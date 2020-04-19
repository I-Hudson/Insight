#include "ispch.h"

#include "Application.h"
#include "Log.h"

#include "Memory/MemoryManager.h"
#include "Module/ModuleManager.h"
#include "Module/WindowModule.h"

namespace Insight
{
	Application::Application()
	{
	}

	Application::~Application()
	{
		m_moduleManager->Shutdown();
		Memory::MemoryManager::DeleteOnStack((Size)m_moduleManager);

		delete m_memoryManager;
	}
	
	void Application::Run()
	{
		Insight::Log::Init();

		m_memoryManager = new Memory::MemoryManager();

		m_moduleManager = Memory::MemoryManager::NewOnStack<Module::ModuleManager>();
		m_moduleManager->Startup();

		Module::ModuleStartupData windowData;
		windowData.ManuallUpdate = true;
		m_windowModule = m_moduleManager->AddModule<Module::WindowModule>(windowData);

		//int* i = Memory::MemoryManager::NewOnFreeList<int>();

		int* ints = Memory::MemoryManager::NewArrOnFreeList<int>(500);
		int* ints1 = Memory::MemoryManager::NewArrOnFreeList<int>(4);

		Memory::MemoryManager::DeleteArrOnFreeList<int>(2, ints1);
		Memory::MemoryManager::DeleteArrOnFreeList<int>(2, ints);


		for (int i = 0; i < 100; i++)
		{
			ints[i] = i;
		}

		bool isRunning = false;

		do
		{
			m_moduleManager->Update(0.0f);

			isRunning = !m_windowModule->GetWindow()->ShouldClose();
			m_windowModule->Update(0.0f);
		} while (isRunning);
	}
}