#include "Editor.h"
#include "Runtime/EntryPoint.h"
#include "Core/Memory.h"

#include "Graphics/Window.h"

namespace Insight
{
	namespace Editor
	{
		void Editor::OnInit()
		{
			Graphics::Window::Instance().SetIcon("./Resources/Insight/default.png");
			Graphics::Window::Instance().Show();
		}

		void Editor::OnDestroy()
		{

		}
	}
}

Insight::App::Engine* CreateApplication()
{
	return NewTracked(Insight::Editor::Editor);
}
