#pragma once
#include "Insight/Core.h"
#include "Insight/Editor/EditorPanel.h"

namespace Insight
{
	class SceneHierarchyPanel : public EditorPanel
	{
	public:
		SceneHierarchyPanel(const Module::EditorModule* editorModule);
		~SceneHierarchyPanel() override;

		virtual void Update(const float& deltaTime) override;

	private:

	};
}