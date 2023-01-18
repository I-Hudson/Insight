#include "Standalone/StandaloneApp.h"

#include "StandaloneModule.h"

#include "Runtime/EntryPoint.h"
#include "Core/Memory.h"
#include "Core/ImGuiSystem.h"

#include "Graphics/Window.h"
#include "Graphics/RenderContext.h"

#include <imgui.h>

namespace Insight
{
	namespace Standalone
	{
		void StandaloneApp::OnInit()
		{
			StandaloneModule::Initialise(GetSystemRegistry().GetSystem<Core::ImGuiSystem>());

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
		}

		void StandaloneApp::OnUpdate()
		{
		}

		void StandaloneApp::OnDestroy()
		{
		}
	}
}

Insight::App::Engine* CreateApplication()
{
	return NewTracked(Insight::Standalone::StandaloneApp);
}
