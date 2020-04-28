#include "ispch.h"

#include "Application.h"
#include "Log.h"
#include "Config/Config.h"

#include "Memory/MemoryManager.h"
#include "Module/ModuleManager.h"
#include "Module/WindowModule.h"
#include "Module/GraphicsModule.h"

#include "Time/Stopwatch.h"
#include "Time/Time.h"

namespace Insight
{
	Application::Application()
	{
		Config::GetInstance().Parse("config.txt");
	}

	Application::~Application()
	{
		m_moduleManager->~ModuleManager();
		Memory::MemoryManager::DeleteOnStack((Size)m_moduleManager);

		delete m_memoryManager;
	}
	
	void AlloBench()
	{
		const Size size = 512_KB;
		int* ints;
		constexpr int loopCount = 100;

		Stopwatch s;
		s.Start();
		for (size_t i = 0; i < loopCount; ++i)
		{
			ints = Memory::MemoryManager::NewArrOnStack<int>(size);
			Memory::MemoryManager::DeleteOnStack((Size)ints);
		}
		s.End();
		std::cout << "Stack -----------------\n";
		std::cout << "Sec: {0}" << s.Sec() << '\n';
		std::cout << "Mil: {0}" << s.Mill() << '\n';
		std::cout << "Nan: {0}" << s.Nano() << '\n';
		std::cout << "Stack -----------------\n";

		Stopwatch sFreeList;
		sFreeList.Start();
		for (size_t i = 0; i < loopCount; ++i)
		{
			ints = Memory::MemoryManager::NewArrOnFreeList<int>(size);
			Memory::MemoryManager::DeleteArrOnFreeList(size, ints);
		}
		sFreeList.End();
		std::cout << "FreeList-----------------\n";
		std::cout << "Sec: {0}" << sFreeList.Sec() << '\n';
		std::cout << "Mil: {0}" << sFreeList.Mill() << '\n';
		std::cout << "Nan: {0}" << sFreeList.Nano() << '\n';
		std::cout << "FreeList-----------------\n";

		Stopwatch sNewDelete;
		sNewDelete.Start();
		for (size_t i = 0; i < loopCount; ++i)
		{
			ints = new int[size];
			delete[] ints;
		}
		sNewDelete.End();
		std::cout << "New/Delete-----------------\n";
		std::cout << "Sec: {0}" << sNewDelete.Sec() << '\n';
		std::cout << "Mil: {0}" << sNewDelete.Mill() << '\n';
		std::cout << "Nan: {0}" << sNewDelete.Nano() << '\n';
		std::cout << "New/Delete-----------------\n";
	}

	void Application::Run()
	{
		m_memoryManager = new Memory::MemoryManager();

		//AlloBench();

		m_moduleManager = Memory::MemoryManager::NewOnStack<Module::ModuleManager>();

		Module::ModuleStartupData windowData;
		windowData.ManuallUpdate = true;
		m_windowModule = m_moduleManager->AddModule<Module::WindowModule>(windowData);

		Module::GraphicsModuleStartupData graphicsData;
		graphicsData.WindowModule = m_windowModule;
		graphicsData.ManuallUpdate = true;
		m_graphicsModule = m_moduleManager->AddModule<Module::GraphicsModule>(graphicsData);

		bool isRunning = false;

		do
		{
			Time::UpdateTime();
			m_moduleManager->Update(Time::GetDeltaTime());

			m_graphicsModule->Update(Time::GetDeltaTime());

			isRunning = !m_windowModule->GetWindow()->ShouldClose();
			m_windowModule->Update(Time::GetDeltaTime());
		} while (isRunning);
	}
}