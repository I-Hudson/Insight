

#include "Engine/Core/Application.h"
#include "Engine/Core/Log.h"
#include "Engine/Config/Config.h"

#include "Engine/Memory/ProfilerMemory.h"
#include "Engine/Memory/Memory.h"

#include "Engine/Module/ModuleManager.h"
#include "Engine/Module/AssetModule.h"
#include "Engine/Module/WindowModule.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/Module/InputModule.h"
#include "Engine/Graphics/ImGuiRenderer.h"
#include "Engine/Event/EventManager.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Instrumentor/Instrumentor.h"
#include "Engine/Serialization/Serializable.h"

#include "Engine/Component/CameraComponent.h"
#include "Engine/Time/Stopwatch.h"
#include "Engine/Time/Time.h"
#include "Engine/Input/Input.h"
#include <glm\ext\matrix_transform.hpp>

#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"

#include "Engine/Threading/TThreadSafe.h"

//#define THREADS

namespace Insight
{
	u64 Application::FrameCount = 0;

	Application::Application()
		: m_state(ApplicationState::Init)
	{
		IS_PROFILE_FUNCTION();

#ifdef IS_DEBUG
		new Core::ProfilerMemory();
#endif

		Config::GetInstance().ParseInFolder("./data/config");

		ASSERT(JS::JobSystemManager::Instance().Init() == JS::JobSystemManager::ReturnCode::Succes);

		m_moduleManager = ::New<Module::ModuleManager>();

		m_moduleManager->AddModule<Module::AssetModule>();

		m_windowModule = m_moduleManager->AddModule<Module::WindowModule>();
		m_windowModule->SetManuallyUpdate(true);

		m_graphicsModule = m_moduleManager->AddModule<Module::GraphicsModule>();
		m_graphicsModule->SetManuallyUpdate(true);
		m_graphicsModule->SetDestroyManually(true);

		m_inputModule = m_moduleManager->AddModule<Module::InputModule>();
		m_inputModule->SetManuallyUpdate(true);

		m_utitledScene = ::New<Scene>();
		m_utitledScene->SetActiveScene();
		m_utitledScene->SetSceneName("Untitled Scene");

		//assertModule->Deserialize();
	}

	Application::~Application()
	{
		IS_PROFILE_FUNCTION();

		m_graphicsModule->WaitForIdle();

		Scene::ActiveScene()->Unload();
		::Delete(m_utitledScene);


		Module::ModuleManager::Instance()->RemoveModule<Module::AssetModule>();
		JS::JobSystemManager::Instance().Shutdown(true);

		Module::ModuleManager::Instance()->RemoveModule<Module::GraphicsModule>();
		::Delete(Module::ModuleManager::Instance());

		while (auto entry = InterlockedPopEntrySList(reinterpret_cast<PSLIST_HEADER>(&__type_info_root_node)))
		{
			free(entry);
		}

#ifdef IS_DEBUG
		delete Core::ProfilerMemory::Instance();
#endif
	}

	void Application::Run()
	{
		bool modulesLoading = true;
		m_state = ApplicationState::Loading;
		while (m_state == ApplicationState::Loading && m_isRunning)
		{
			modulesLoading = false;
#ifdef IMGUI_ENABLED
			if (ImGuiRenderer::IsInitialised())
			{
				ImGuiRenderer::Instance()->NewFrame();
			}
#endif
			for (auto& mod : m_moduleManager->GetAllModules())
			{
				if (mod->GetState() == Module::ModuleState::Init)
				{
					mod->OnCreate();
				}
				if (mod->GetState() == Module::ModuleState::Loading)
				{
					modulesLoading = true;
				}
				mod->Update(0.0f);
			}

			if (!modulesLoading)
			{
				if (m_isRunning)
				{
					Create();
				}
			}
			m_isRunning = !m_windowModule->GetWindow()->ShouldClose();
		}

#ifdef THREADS
		m_updateThreadState = UpdateThreadState::SAME_FRMAE;
		m_renderThread = std::thread(&Application::RenderLoop, this);
#endif

		uint32_t m_loopCount = 0;
		uint32_t m_frameCount = 0;
		float deltaTime = 0.0f;

		while (m_isRunning)
		{
			++FrameCount;
			++m_loopCount;

			IS_PROFILE_FRAME("MainThread");

			IS_PROFILE_SCOPE("UPDATE_LOOP");
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
				JS::JobSystemManager::Instance().Update(64);

				m_inputModule->Update(deltaTime);
				m_windowModule->Update(deltaTime);

#ifdef IMGUI_ENABLED
				ImGuiRenderer::Instance()->NewFrame();
#endif
				m_moduleManager->Update(deltaTime);
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
		};

#ifdef THREADS
		m_renderThread.join();
#endif

		IS_CORE_INFO("UPDATE LOOP COUNT: {0}", m_loopCount);
		IS_CORE_INFO("UPDATE FRAME COUNT: {0}", m_frameCount);
		m_graphicsModule->WaitForIdle();
	}

	void Application::RenderLoop()
	{
		uint32_t m_loopCount = 0;
		uint32_t m_frameCount = 0;

		IS_PROFILE_THREAD("Render Thread");
		//Optick::Category::Type optickCat = (Optick::Category::Type)((uint32_t)Category::AI);

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