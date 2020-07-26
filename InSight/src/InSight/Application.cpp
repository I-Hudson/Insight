#include "ispch.h"

#include "Application.h"
#include "Log.h"
#include "Config/Config.h"

#include "Memory/MemoryManager.h"
#include "Module/ModuleManager.h"
#include "Module/WindowModule.h"
#include "Module/GraphicsModule.h"
#include "Module/InputModule.h"
#include "Module/EntityModule.h"

#include "Insight/Camera.h"

#include "Time/Stopwatch.h"
#include "Time/Time.h"
#include "Input/Input.h"
#include <glm\ext\matrix_transform.hpp>

namespace Insight
{
	Application::Application()
	{
		Config::GetInstance().Parse("config.txt");
		
		m_memoryManager = Memory::MemoryManager::CreateWithoutMemoryManager();
		
		m_moduleManager = Module::ModuleManager::Create();
		
		Module::ModuleStartupData windowData;
		windowData.ManuallUpdate = true;
		m_windowModule = m_moduleManager->AddModule<Module::WindowModule>(windowData);
		
		Module::GraphicsModuleStartupData graphicsData;
		graphicsData.WindowModule = m_windowModule;
		graphicsData.ManuallUpdate = true;
		m_graphicsModule = m_moduleManager->AddModule<Module::GraphicsModule>(graphicsData);
		
		Module::InputModuleData inputData;
		inputData.WindowModule = m_windowModule;
		inputData.ManuallUpdate = true;
		m_inputModule = m_moduleManager->AddModule<Module::InputModule>(inputData);
		
		m_moduleManager->AddModule<Module::EntityModule>();
		
		m_mainCamera = CreateUniquePtr<Camera>();
		m_mainCamera->SetProjMatrix(90.0f, CameraAspect::CurrentWindowSize, 0.1f, 1000.0f);
		m_mainCamera->SetViewMatrix(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
		
		m_graphicsModule->SetMainCamera(m_mainCamera.get());
	}

	Application::~Application()
	{
		Module::ModuleManager::Destroy();

		Memory::MemoryManager::DestroyWithoutMemoryManager();

		while (auto entry = InterlockedPopEntrySList(reinterpret_cast<PSLIST_HEADER>(&__type_info_root_node)))
		{
			free(entry);
		}
	}
	
	void AllcoBench()
	{
		const Size size = 512_KB;
		int* ints;
		constexpr int loopCount = 1000;

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

		Stopwatch sFreeSingle;
		sFreeSingle.Start();
		for (size_t i = 0; i < loopCount; ++i)
		{
			int* in = NEW_ON_HEAP(int);
			DELETE_ON_HEAP(in);
		}
		sFreeSingle.End();
		std::cout << "New/Delete-----------------\n";
		std::cout << "Sec: {0}" << sFreeSingle.Sec() << '\n';
		std::cout << "Mil: {0}" << sFreeSingle.Mill() << '\n';
		std::cout << "Nan: {0}" << sFreeSingle.Nano() << '\n';
		std::cout << "New/Delete-----------------\n";

		Stopwatch sNewDeleteSingle;
		sNewDeleteSingle.Start();
		for (size_t i = 0; i < loopCount; ++i)
		{
			int* in = new int;
			delete in;
		}
		sNewDeleteSingle.End();
		std::cout << "New/Delete-----------------\n";
		std::cout << "Sec: {0}" << sNewDeleteSingle.Sec() << '\n';
		std::cout << "Mil: {0}" << sNewDeleteSingle.Mill() << '\n';
		std::cout << "Nan: {0}" << sNewDeleteSingle.Nano() << '\n';
		std::cout << "New/Delete-----------------\n";
	}

	void Application::Run()
	{
		//AllcoBench();
		Create();

		bool isRunning = false;

		do
		{
			Time::UpdateTime();
			m_moduleManager->Update(Time::GetDeltaTime());

			Update(Time::GetDeltaTime());
			Draw();

			m_inputModule->Update(Time::GetDeltaTime());
			m_mainCamera->Update(Time::GetDeltaTime());

			m_graphicsModule->Update(Time::GetDeltaTime());

			isRunning = !m_windowModule->GetWindow()->ShouldClose();
			m_windowModule->Update(Time::GetDeltaTime());
		} while (isRunning);
	}
}