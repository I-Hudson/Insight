#include "ispch.h"
#include "EditorModule.h"
#include "Editor/EditorWindow.h"

#include "Editor/SceneHierarchyPanel.h"
#include "Editor/TopBarMenu.h"
#include "Editor/Windows/AssetWindow.h"

#if defined(IS_EDITOR)
	namespace Module
	{
		EditorModule::EditorModule()
		{
			Editor::EditorDrawerRegistry::GetTypes();
		}

		EditorModule::~EditorModule()
		{
			for (auto it = m_editorPanels.begin(); it != m_editorPanels.end(); ++it)
			{
				::Delete(it->second);
			}

			m_editorPanels.clear();
		}

		void EditorModule::OnCreate()
		{
			AddEditorPanel<Editor::SceneHierarchyPanel>();
			AddEditorPanel<Editor::TopBarMenu>();
			AddEditorPanel<Editor::AssetWindow>();
		}

		void EditorModule::Update(const float& deltaTime)
		{
			IS_PROFILE_FUNCTION();

			for (auto it = m_editorPanels.begin(); it != m_editorPanels.end(); ++it)
			{
				it->second->Update(deltaTime);
			}
		}
	}
#endif