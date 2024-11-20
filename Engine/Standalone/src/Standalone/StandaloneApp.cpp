#include "Standalone/StandaloneApp.h"

#include "StandaloneModule.h"

#include "Core/Memory.h"
#include "Core/ImGuiSystem.h"
#include "Core/EnginePaths.h"

#include "Graphics/Window.h"
#include "Graphics/RenderContext.h"
#include "Renderpass.h"

#include "Asset/AssetRegistry.h"
#include "Serialisation/Serialisers/JsonSerialiser.h"

#include "Runtime/EntryPoint.h"

#ifdef IS_PACKAGE_BUILD
#include "Generated/ProjectInitialise.gen.h"
#endif

namespace Insight
{
	namespace Standalone
	{
		void StandaloneApp::OnInit()
		{
#ifdef IS_PACKAGE_BUILD
			ProjectModuleInitialise(GetSystemRegistry().GetSystem<Core::ImGuiSystem>());
#endif

			StandaloneModule::Initialise(GetSystemRegistry().GetSystem<Core::ImGuiSystem>());

			Runtime::AssetRegistry::Instance().LoadAssetPackage("BuiltContent.zip");
			Runtime::AssetRegistry::Instance().LoadAssetPackage("ProjectAssets.zip");

			std::string windowTitle = "Insight Standalone";
#ifdef IS_DEBUG
			windowTitle += " Debug ";
#elif IS_RELEASE
			windowTitle += " Release ";
#endif
			windowTitle += "(";
			windowTitle += Graphics::GraphicsAPIToString(Graphics::RenderContext::Instance().GetGraphicsAPI());
			windowTitle += ")";

			Graphics::Window::Instance().SetTite(windowTitle);
			Graphics::Window::Instance().SetIcon("./Resources/Insight/default.png");
			Graphics::Window::Instance().Show();

			m_gameRenderpass = New<Graphics::Renderpass>();
			m_gameRenderpass->Create();

			std::vector<u8> runtimeSettingsData = Runtime::AssetRegistry::Instance().LoadAssetData("RuntimeSettings.json");
			Serialisation::JsonSerialiser runtimeSettingsSerialsier(true);
			if (runtimeSettingsSerialsier.Deserialise(runtimeSettingsData))
			{
				Runtime::RuntimeSettings& runtimeSettings = Runtime::RuntimeSettings::Instance();
				runtimeSettings.Deserialise(&runtimeSettingsSerialsier);
				Insight::Runtime::World* loadedWorld = Runtime::WorldSystem::Instance().LoadWorld(runtimeSettings.InitialWorldPath);
				if (loadedWorld)
				{
					loadedWorld->SetWorldState(Runtime::WorldStates::Running);
				}
			}
		}

		void StandaloneApp::OnUpdate()
		{
			const Graphics::Window& window = Graphics::Window::Instance();
			const u32 consoleHeight = window.GetHeight() * 0.3f;
			Core::Console::Instance().Render(0, window.GetHeight() - consoleHeight, window.GetWidth(), consoleHeight);
		}

		void StandaloneApp::OnDestroy()
		{
			m_gameRenderpass->Destroy();
			Delete(m_gameRenderpass);
		}
	
		void StandaloneApp::OnRender()
		{
			m_gameRenderpass->FrameSetup();
			m_gameRenderpass->RenderMainPasses(true);
			m_gameRenderpass->RenderSwapchain(true);
			m_gameRenderpass->RenderPostprocessing();
		}
	}
}

Insight::App::Engine* CreateApplication()
{
	return NewTracked(Insight::Standalone::StandaloneApp);
}
