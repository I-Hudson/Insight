#pragma once

#include "Runtime/Engine.h"

#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/MenuBar.h"
#include "Editor/TypeDrawers/TypeDrawerRegister.h"

#include "Renderpass.h"

namespace Insight
{
	namespace Editor
	{
		class Editor : public Insight::App::Engine
		{
		public:

			virtual void OnInit() override;
			virtual void OnUpdate() override;
			virtual void OnRender() override;
			virtual void OnDestroy() override;

		private:
			Graphics::Renderpass* m_gameRenderpass;
			MenuBar m_menuBar;
			EditorWindowManager m_editorWindowManager;
			TypeDrawerRegister m_typeDrawerRegister;
		};
	}
}