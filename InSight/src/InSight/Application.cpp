#include "ispch.h"

#include "Application.h"
#include "Log.h"
#include "Config/Config.h"

#include "Memory/MemoryManager.h"
#include "Module/ModuleManager.h"
#include "Module/AssetModule.h"
#include "Module/WindowModule.h"
#include "Module/GraphicsModule.h"
#include "Module/InputModule.h"
#include "Module/EntityModule.h"
#include "Insight/Renderer/ImGuiRenderer.h"
#include "Insight/Event/EventManager.h"

#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Serialization/Serializable.h"

#include <ppltasks.h>

#include "Insight/Camera.h"

#include "Time/Stopwatch.h"
#include "Time/Time.h"
#include "Input/Input.h"
#include <glm\ext\matrix_transform.hpp>

namespace Insight
{
	void PrintFunction()
	{
		IS_PROFILE_FUNCTION();

		IS_CORE_INFO("TEST");
	}

	Application::Application()
	{
		IS_PROFILE_FUNCTION();

		Config::GetInstance().Parse("config.txt");
		
		m_memoryManager = Memory::MemoryManager::CreateWithoutMemoryManager();
		
		m_moduleManager = Module::ModuleManager::Create();
		
		m_moduleManager->AddModule<Module::AssetModule>();

		m_windowModule = m_moduleManager->AddModule<Module::WindowModule>();
		m_windowModule->SetManuallyUpdate(true);

		m_graphicsModule = m_moduleManager->AddModule<Module::GraphicsModule>(m_windowModule);
		m_graphicsModule->SetManuallyUpdate(true);

		m_inputModule = m_moduleManager->AddModule<Module::InputModule>(m_windowModule);
		m_inputModule->SetManuallyUpdate(true);

		m_moduleManager->AddModule<Module::EntityModule>();

		m_moduleManager->GetModule<Module::AssetModule>()->AddDependency(m_graphicsModule);

		m_mainCamera = CreateUniquePtr<Camera>();
		m_mainCamera->SetProjMatrix(90.0f, CameraAspect::CurrentWindowSize, 0.1f, 1000.0f);
		m_mainCamera->SetViewMatrix(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
		
		m_graphicsModule->SetMainCamera(m_mainCamera.get());

		m_moduleManager->GetModule<Module::AssetModule>()->Deserialize();

		Concurrency::create_task([]() {IS_CORE_INFO("Thread for task"); }).wait();

		IS_CORE_INFO("ALL TASKS ARE COMPLETED!");
	}

	Application::~Application()
	{
		IS_PROFILE_FUNCTION();

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

	void TestFunc(Camera* cam)
	{
		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::SliderFloat("Main Camera FOV: ", &cam->GetFov(), 0.1f, 120.0f);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		if (Input::KeyDown(KEY_LEFT_CONTROL) && Input::KeyDown(KEY_S))
		{
			Serialization::SerializableRegistry::SerializeAll();
		}
		if (Input::KeyDown(KEY_LEFT_CONTROL) && Input::KeyDown(KEY_F))
		{
			Serialization::SerializableRegistry::DeserializeAll();
			EventManager::Dispatch(EventType::Deserialize, DeserializeEvent());
		}
	}

	void Application::Run()
	{
		//AllcoBench();
		Create();

		bool isRunning = false;

		do
		{
			IS_PROFILE_FUNCTION();

			Time::UpdateTime();
			m_moduleManager->Update(Time::GetDeltaTime());

			Render::ImGuiRenderer::GetInstance()->NewFrame();

			static float f = 0.0f;
			static int counter = 0;

			bool show_demo_window = true;
			bool show_another_window = true;

			TestFunc(m_mainCamera.get());
			
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