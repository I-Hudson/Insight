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
#include "Module/EditorModule.h"
#include "Insight/Renderer/ImGuiRenderer.h"
#include "Insight/Event/EventManager.h"
#include "Insight/Scene/Scene.h"

#include "Insight/RTTI/RTTI.h"

#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Serialization/Serializable.h"

#include "Insight/Component/CameraComponent.h"
#include "Time/Stopwatch.h"
#include "Time/Time.h"
#include "Input/Input.h"
#include <glm\ext\matrix_transform.hpp>
#include "misc/cpp/imgui_stdlib.h"

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

		RTTI::RTTI::Create();
		
		m_moduleManager = Module::ModuleManager::Create();
		
		m_moduleManager->AddModule<Module::AssetModule>();

		m_windowModule = m_moduleManager->AddModule<Module::WindowModule>();
		m_windowModule->SetManuallyUpdate(true);

		m_graphicsModule = m_moduleManager->AddModule<Module::GraphicsModule>(m_windowModule);
		m_graphicsModule->SetManuallyUpdate(true);

		m_inputModule = m_moduleManager->AddModule<Module::InputModule>(m_windowModule);
		m_inputModule->SetManuallyUpdate(true);

		m_moduleManager->GetModule<Module::AssetModule>()->AddDependency(m_graphicsModule);
		
		m_moduleManager->GetModule<Module::AssetModule>()->Deserialize();

		m_moduleManager->AddModule<Module::EditorModule>();

		IS_CORE_INFO("ALL TASKS ARE COMPLETED!");
	}

	Application::~Application()
	{
		IS_PROFILE_FUNCTION();

		Module::ModuleManager::Destroy();

		RTTI::RTTI::Destroy();

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

	void TestFunc(CameraComponent* cam)
	{
#if defined(IS_EDITOR) && defined(IMGUI_ENABLED)
		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		float cameraFOV = cam != nullptr ? cam->GetFov() : 0;
		if (ImGui::SliderFloat("Main Camera FOV: ", &cameraFOV, 0.1f, 120.0f))
		{
			if (cam != nullptr)
			{
				cam->SetFov(cameraFOV);
			}
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
#endif
	}

	void Application::Run()
	{
		//AllcoBench();
		Create();

		bool isRunning = false;

		do
		{
			IS_PROFILE_FUNCTION();

#if defined(IS_EDITOR) && defined(IMGUI_ENABLED)
			ImGuiRenderer::GetInstance()->NewFrame();	

			std::string sceneFileName = Scene::ActiveScene()->GetSceneName();
			ImGui::Begin("Scene");
			ImGui::InputText("Scene Name", &sceneFileName);
			ImGui::End();

			Scene::ActiveScene()->SetSceneName(sceneFileName);

			ImGui::BeginMainMenuBar();

			if (ImGui::BeginMenu("Scene"))
			{
				if (ImGui::MenuItem("Save Scene"))
				{
					Scene::ActiveScene()->Save();
				}

				if (ImGui::MenuItem("Load Scene"))
				{
					Scene::ActiveScene()->Load(sceneFileName);
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Save Model Library"))
			{
				tinyxml2::XMLDocument doc;
				tinyxml2::XMLNode* models = doc.NewElement("Models");
				Insight::Library::ModelLibrary::GetInstance()->Serialize(models, &doc);
				doc.InsertEndChild(models);
				doc.SaveFile("ModelLibrary.xml");
			}
			ImGui::EndMainMenuBar();
#endif

			Time::UpdateTime();

			m_moduleManager->Update(Time::GetDeltaTime());

			static float f = 0.0f;
			static int counter = 0;

			bool show_demo_window = true;
			bool show_another_window = true;

			TestFunc(Scene::ActiveScene()->FindFirstComponent<CameraComponent>());
			
			Update(Time::GetDeltaTime());
			Draw();

			m_inputModule->Update(Time::GetDeltaTime());

			m_graphicsModule->Update(Time::GetDeltaTime());

			isRunning = !m_windowModule->GetWindow()->ShouldClose();
			m_windowModule->Update(Time::GetDeltaTime());

			OnFrameEnd();

		} while (isRunning);
	}
}