#pragma once
#include "Insight/Core/Core.h"
#include "Editor/EditorWindow.h"

class Entity;

namespace Insight
{
	namespace Editor
	{
		class SceneHierarchyPanel : public EditorWindow
		{
		public:
			SceneHierarchyPanel(const SharedPtr<Module::EditorModule> editorModule);
			~SceneHierarchyPanel() override;

			virtual void Update(const float& deltaTime) override;

		private:
			void DrawEntityTreeView(WeakPtr<Entity> entity, bool& newEntitySelected);
			int GetTreeNodeFlags(WeakPtr<Entity> entity);

			void DrawCompoentPanel(WeakPtr<Entity> entity);

			SharedPtr<Entity> m_selectedEntity;
			bool m_openContextPopup;
			bool m_openAddComponentPopup;
		};
	}
}