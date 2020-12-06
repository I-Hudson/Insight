#pragma once
#include "Insight/Core.h"
#include "Insight/Editor/EditorPanel.h"

class Entity;

namespace Insight
{
	namespace Editor
	{
		class SceneHierarchyPanel : public EditorPanel
		{
		public:
			SceneHierarchyPanel(const Module::EditorModule* editorModule);
			~SceneHierarchyPanel() override;

			virtual void Update(const float& deltaTime) override;

		private:
			void DrawEntityTreeView(Entity* entity, bool& newEntitySelected);
			int GetTreeNodeFlags(Entity* entity);

			void DrawCompoentPanel(Entity* entity);

			Entity* m_selectedEntity;
		};
	}
}