#pragma once

#include "Editor/Defines.h"

#include "Runtime/Engine.h"

#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/MenuBar.h"
#include "Editor/TypeDrawers/TypeDrawerRegister.h"
#include "Editor/HotReload/HotReloadSystem.h"

#include "Renderpass.h"

namespace Insight
{
	namespace Editor
	{
		class IS_EDITOR Editor : public Insight::App::Engine, public Serialisation::ISerialisable
		{
		public:
			IS_SERIALISABLE_H(Editor)

			virtual void OnInit() override;
			virtual void OnPostInit() override;

			virtual void OnUpdate() override;
			virtual void OnRender() override;
			virtual void OnDestroy() override;

		private:
			Graphics::Renderpass* m_gameRenderpass;
			MenuBar m_menuBar;
			EditorWindowManager m_editorWindowManager;
			TypeDrawerRegister m_typeDrawerRegister;
			HotReloadSystem m_hotReloadSystem;

			using EditorSettingsSerialiser = Serialisation::JsonSerialiser;
			constexpr static const char* c_EditorSettingsFileName = "InsightEditorSettings";
		};
	}

	OBJECT_SERIALISER(Editor::Editor, 1, 
		SERIALISE_OBJECT(Editor::EditorWindowManager, m_editorWindowManager, 1, 0)
	)
}