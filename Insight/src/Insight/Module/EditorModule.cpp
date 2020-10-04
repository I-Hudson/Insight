#include "ispch.h"
#include "EditorModule.h"
#include "Insight/Editor/EditorPanel.h"

#include "Insight/Editor/SceneHierarchyPanel.h"

namespace Insight
{
	namespace Module
	{
		EditorModule::EditorModule()
		{
			AddEditorPanel<SceneHierarchyPanel>();
		}

		EditorModule::~EditorModule()
		{
			for (auto it = m_editorPanels.begin(); it != m_editorPanels.end(); ++it)
			{
				DELETE_ON_HEAP(it->second);
			}

			m_editorPanels.clear();
		}

		void EditorModule::Update(const float& deltaTime)
		{
			for (auto it = m_editorPanels.begin(); it != m_editorPanels.end(); ++it)
			{
				it->second->Update(deltaTime);
			}
		}
	}
}