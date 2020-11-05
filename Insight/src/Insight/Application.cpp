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

#ifdef IS_EDITOR
		m_moduleManager->AddModule<Module::EditorModule>();
#endif

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

			Time::UpdateTime();

			IS_CORE_INFO("FPS: {0}", 1.0f / Time::GetDeltaTime());
			IS_CORE_INFO("Frame Time: {0}", Time::GetDeltaTime());

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