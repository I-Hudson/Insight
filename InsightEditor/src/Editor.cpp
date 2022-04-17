#include "Editor.h"
#include "App/EntryPoint.h"
#include "Core/Memory.h"

namespace Insight
{
	namespace Editor
	{
		void Editor::OnInit()
		{

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