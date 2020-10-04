#include "ispch.h"
#include "Insight/Editor/SceneHierarchyPanel.h"
#include "Insight/Scene/Scene.h"
#include "Insight/Entitiy/Entity.h"

namespace Insight
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Module::EditorModule* editorModule)
		: EditorPanel(editorModule)
	{
		SET_PANEL_NAME(SceneHierarchyPanel);
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
	}

	void SceneHierarchyPanel::Update(const float& deltaTime)
	{
		auto entites = Scene::ActiveScene()->GetEntites();

		for (auto it = entites.begin(); it != entites.end(); ++it)
		{
			auto components = (*it)->GetAllComponents();


		}
	}
}