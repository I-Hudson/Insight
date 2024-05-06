#pragma once

#include "Editor/Defines.h"

#include "Runtime/Engine.h"

#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/MenuBar.h"
#include "Editor/TypeDrawers/TypeDrawerRegister.h"
#include "Editor/HotReload/HotReloadSystem.h"
#include "Editor/Build/BuildSystem.h"

#include "Editor/ContentFolderListener.h"

#include "Serialisation/Serialisers/JsonSerialiser.h"

#include "Renderpass.h"

namespace Insight
{
	namespace Runtime
	{
		class IAssetPackage;
	}

	namespace Editor
	{
		class IS_EDITOR Editor : public Insight::App::Engine, public Serialisation::ISerialisable
		{
		public:
			Editor();
			virtual ~Editor() override;

			constexpr static const char* c_ProjectAssetPackageName = "ProjectAssets";

			IS_SERIALISABLE_H(Editor)

			virtual void OnPreInit() override;
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
			BuildSystem m_buildSystem;

			efsw::FileWatcher m_fileWatcher;
			ContentFolderListener m_contentListener;

			Runtime::IAssetPackage* m_engineAssetPackage;
			Runtime::IAssetPackage* m_projectAssetPackage;

			using EditorSettingsSerialiser = Serialisation::JsonSerialiser;
			constexpr static const char* c_EditorSettingsFileName = "InsightEditorSettings.isEditorSettings";
		};
	}

	OBJECT_SERIALISER(Editor::Editor, 2, 
		SERIALISE_OBJECT(Editor::EditorWindowManager, m_editorWindowManager, 1, 0)
		SERIALISE_OBJECT(Editor::BuildSystem, m_buildSystem, 2, 0)
	)
}