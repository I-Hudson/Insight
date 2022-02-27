#include "Editor.h"
#include "Core/EntryPoint.h"

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

Insight::Core::Engine* CreateApplication()
{
	return new Insight::Editor::Editor();
}