#include "Editor.h"
#include "App/EntryPoint.h"

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
	return new Insight::Editor::Editor();
}