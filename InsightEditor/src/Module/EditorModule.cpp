
#include "Module/EditorModule.h"
#include "Editor/EditorWindow.h"

#include "Editor/SceneHierarchyPanel.h"
#include "Editor/TopBarMenu.h"
#include "Editor/Windows/AssetWindow.h"
#include "Editor/Windows/SceneWindow.h"

#include "Engine/Graphics/ImGuiRenderer.h"

#if defined(IS_EDITOR)
namespace Insight::Module
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
		AddEditorPanel<Insight::Editor::SceneHierarchyPanel>();
		AddEditorPanel<Insight::Editor::TopBarMenu>();
		AddEditorPanel<Insight::Editor::AssetWindow>();
		AddEditorPanel<Insight::Editor::SceneWindow>();
	}

	void EditorModule::Update(const float& deltaTime)
	{
		IS_PROFILE_FUNCTION();

		if (ImGuiRenderer::Instance()->IsInit())
		{
			for (auto it = m_editorPanels.begin(); it != m_editorPanels.end(); ++it)
			{
				it->second->Update(deltaTime);
			}
		}
	}
}
#endif