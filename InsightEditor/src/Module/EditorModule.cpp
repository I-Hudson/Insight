#include "ispch.h"
#include "EditorModule.h"
#include "Editor/EditorPanel.h"

#include "Editor/SceneHierarchyPanel.h"

#if defined(IS_EDITOR)
namespace Insight
{
	namespace Module
	{
		EditorModule::EditorModule()
		{
			SetInstancePtr(this);
			Editor::EditorDrawerRegistry::GetTypes();
		}

		EditorModule::~EditorModule()
		{
			for (auto it = m_editorPanels.begin(); it != m_editorPanels.end(); ++it)
			{
				it->second.reset();
			}

			m_editorPanels.clear();
			ClearPtr();
		}

		void EditorModule::OnCreate()
		{
			AddEditorPanel<Editor::SceneHierarchyPanel>();
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
}
#endif