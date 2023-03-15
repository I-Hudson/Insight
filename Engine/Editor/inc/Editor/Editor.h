#pragma once

#include "Runtime/Engine.h"

#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/MenuBar.h"

namespace Insight
{
	namespace Editor
	{
		class Editor : public Insight::App::Engine
		{
		public:

			virtual void OnInit() override;
			virtual void OnUpdate() override;
			virtual void OnDestroy() override;

		private:
			MenuBar m_menuBar;
			EditorWindowManager m_editorWindowManager;
		};
	}
}