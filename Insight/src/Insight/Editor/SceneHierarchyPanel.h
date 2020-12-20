#pragma once
#include "Insight/Core/Core.h"
#include "Insight/Editor/EditorPanel.h"

class Entity;

namespace Insight
{
	namespace Editor
	{
		class SceneHierarchyPanel : public EditorPanel
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
		};
	}
}