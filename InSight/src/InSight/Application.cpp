#include "Application.h"

#include "Module/ModuleManager.h"
#include "Module/LogModule.h"
#include "Module/WindowModule.h"

namespace Insight
{
	Application::Application()
	{
	}

	Application::~Application()
	{
		m_moduleManager->Shutdown();
	}
	
	void Application::Run()
	{
		m_moduleManager = new Module::ModuleManager();
		m_moduleManager->Startup();
		m_moduleManager->AddModule<Module::LogModule>();

		Module::ModuleStartupData windowData;
		windowData.ManuallUpdate = true;
		m_moduleManager->AddModule<Module::WindowModule>(windowData);

		bool isRunning = false;

		do
		{
			m_moduleManager->Update(0.0f);

			isRunning = !Module::WindowModule::GetWindow()->ShouldClose();
			m_moduleManager->GetModule<Module::WindowModule>()->Update(0.0f);
		} while (isRunning);


		IS_CORE_INFO("Core loop shuting down");
	}
}