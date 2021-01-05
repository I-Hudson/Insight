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

#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"

#include "Threading/TThreadSafe.h"

//#define THREADS

namespace Insight
{
	Application::Application()
	{
		IS_PROFILE_FUNCTION();

		Config::GetInstance().ParseInFolder("./data/config");
		
		m_memoryManager = Memory::MemoryManager::CreateWithoutMemoryManager();

		RTTI::RTTI::Create();
		
		m_moduleManager = Module::ModuleManager::Create();
		
		auto assertModule = m_moduleManager->AddModule<Module::AssetModule>();

		m_windowModule = m_moduleManager->AddModule<Module::WindowModule>();
		m_windowModule->SetManuallyUpdate(true);

		m_graphicsModule = m_moduleManager->AddModule<Module::GraphicsModule>(m_windowModule);
		m_graphicsModule->SetManuallyUpdate(true);

		m_inputModule = m_moduleManager->AddModule<Module::InputModule>(m_windowModule);
		m_inputModule->SetManuallyUpdate(true);

		m_moduleManager->GetModule<Module::AssetModule>()->AddDependency(m_graphicsModule);

		assertModule->Deserialize();
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

	void Application::Run()
	{
		//AllcoBench();
		Create();

#ifdef THREADS
			m_updateThreadState = UpdateThreadState::SAME_FRMAE;
			m_renderThread = std::thread(&Application::RenderLoop, this);
#endif

		uint32_t m_loopCount = 0;
		uint32_t m_frameCount = 0;
		float deltaTime = 0.0f;
		Scene untitledScene;
		untitledScene.SetActiveScene();
		untitledScene.SetSceneName("Untitled Scene");

		while (m_isRunning)
		{
			IS_PROFILE_FRAME("MainThread");

			IS_PROFILE_SCOPE("UPDATE_LOOP");
			{
				IS_PROFILE_SCOPE("UPDATE_LOOP_START");
			}
#ifdef THREADS
			if (m_updateThreadState == UpdateThreadState::SAME_FRMAE || m_updateThreadState == UpdateThreadState::ONE_FRAME_AHEAD)
#endif
			{
#ifdef THREADS
				m_mutex.lock();
				m_triggerRender = false;
				m_mutex.unlock();
#endif
				Time::UpdateTime();
				deltaTime = Time::GetDeltaTime();

				m_inputModule->Update(deltaTime);
				m_windowModule->Update(deltaTime);

				//IS_CORE_INFO("FPS: {0}", 1.0 / deltaTime);
				//IS_CORE_INFO("Frame Time: {0}", deltaTime);

#ifdef IMGUI_ENABLED
				Insight::ImGuiRenderer::Instance()->NewFrame();
#endif
//#if defined(IS_EDITOR) && defined(IMGUI_ENABLED)
//				
//				std::string sceneFileName = Scene::ActiveScene()->GetSceneName();
//				ImGui::Begin("Scene");
//				ImGui::InputText("Scene Name", &sceneFileName);
//				ImGui::End();
//
//				Scene::ActiveScene()->SetSceneName(sceneFileName);
//
//				ImGui::BeginMainMenuBar();
//
//				if (ImGui::BeginMenu("Scene"))
//				{
//					if (ImGui::MenuItem("Save Scene"))
//					{
//						Scene::ActiveScene()->Save();
//					}
//
//					if (ImGui::MenuItem("Load Scene"))
//					{
//						Scene::ActiveScene()->Load(sceneFileName);
//					}
//					ImGui::EndMenu();
//				}
//
//				if (ImGui::MenuItem("Save Model Library"))
//				{
//					//tinyxml2::XMLDocument doc;
//					//tinyxml2::XMLNode* models = doc.NewElement("Models");
//					//Insight::Library::ModelLibrary::Instance()->Serialize(models, &doc);
//					//doc.InsertEndChild(models);
//					//doc.SaveFile("ModelLibrary.xml");
//				}
//				ImGui::EndMainMenuBar();
//#endif
				m_moduleManager->Update(deltaTime);

				//TestFunc(Scene::ActiveScene()->FindFirstComponent<CameraComponent>());

				Update(deltaTime);
				Scene::ActiveScene()->OnUpdate(deltaTime);

				Draw();
#ifndef THREADS
				m_graphicsModule->Update(deltaTime);
#endif

#ifdef THREADS
				m_mutex.lock();
#endif
				m_isRunning = !m_windowModule->GetWindow()->ShouldClose();
				//if (m_windowModule->GetWindow()->ShouldClose())
				//{
				//	break;
				//}
#ifdef THREADS
				m_renderComplete = false;
				m_triggerRender = true;
				m_updateThreadState = (UpdateThreadState)((int)m_updateThreadState + 1);
				// Transfer all data to render thread.
				m_mutex.unlock();
#endif	
				OnFrameEnd();
				++m_frameCount;
			}
			++m_loopCount;
			{
				IS_PROFILE_SCOPE("UPDATE_LOOP_END");
			}
		};

#ifdef THREADS
		m_renderThread.join();
#endif

		IS_CORE_INFO("UPDATE LOOP COUNT: {0}", m_loopCount);
		IS_CORE_INFO("UPDATE FRAME COUNT: {0}", m_frameCount);
	}

	void Application::RenderLoop()
	{
		uint32_t m_loopCount = 0;
		uint32_t m_frameCount = 0;

		IS_PROFILE_THREAD("Render Thread");
		//Optick::Category::Type optickCat = (Optick::Category::Type)((uint32_t)Insight::Category::AI);

		while (m_isRunning)
		{
			IS_PROFILE_FUNCTION();
			if (m_triggerRender)
			{
				m_graphicsModule->Update(Time::GetDeltaTime());
				m_mutex.lock();
				m_renderComplete = true;
				m_triggerRender = false;
				m_updateThreadState = UpdateThreadState::SAME_FRMAE;
				m_mutex.unlock();
				++m_frameCount;
			}
			++m_loopCount;
		}

		IS_CORE_INFO("RENDER LOOP COUNT: {0}", m_loopCount);
		IS_CORE_INFO("RENDER FRAME COUNT: {0}", m_frameCount);
	}
}