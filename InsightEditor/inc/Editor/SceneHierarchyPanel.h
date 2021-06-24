#pragma once
#include "Engine/Core/Core.h"
#include "Editor/EditorWindow.h"
#include "Engine/Entity/Entity.h"

namespace Insight::Editor
{
	class SceneHierarchyPanel : public EditorWindow
	{
	public:
		SceneHierarchyPanel(const Module::EditorModule* editorModule);
		~SceneHierarchyPanel() override;

		virtual void Update(const float& deltaTime) override;

		Entity GetSelectedEntity() const { return m_selectedEntity; }

	private:
		void DrawEntityTreeView(Entity& entity, bool& newEntitySelected);
		int GetTreeNodeFlags(Entity& entity);

		void DrawCompoentPanel(Entity& entity);

		Entity m_selectedEntity;
		bool m_openContextPopup;
		bool m_openAddComponentPopup;
	};
}