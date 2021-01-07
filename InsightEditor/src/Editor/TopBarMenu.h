#pragma once
#include "Insight/Core/Core.h"
#include "Editor/EditorPanel.h"
#include "ImGuiFileBrowser.h"

namespace Insight
{
	namespace Editor
	{
		class TopBarMenu : public EditorPanel
		{
		public:
			TopBarMenu(const SharedPtr<Module::EditorModule> editorModule);
			~TopBarMenu() override;

			virtual void Update(const float& deltaTime) override;

		private:
			imgui_addons::ImGuiFileBrowser m_fileDialog;
		};
	}
}