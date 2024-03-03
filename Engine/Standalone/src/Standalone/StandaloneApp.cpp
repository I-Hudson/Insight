#include "Standalone/StandaloneApp.h"

#include "StandaloneModule.h"

#include "Runtime/EntryPoint.h"
#include "Core/Memory.h"
#include "Core/ImGuiSystem.h"
#include "Core/EnginePaths.h"

#include "Graphics/Window.h"
#include "Graphics/RenderContext.h"
#include "Renderpass.h"

#include "Asset/AssetRegistry.h"

#include <imgui.h>

namespace Insight
{
	namespace Standalone
	{
		void StandaloneApp::OnInit()
		{
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
		}

		void StandaloneApp::OnUpdate()
		{
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
