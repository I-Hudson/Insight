#pragma once
#include "Insight/Core/Core.h"
#include "Editor/EditorWindow.h"
#include "ImGuiFileBrowser.h"

namespace Insight
{
	namespace Editor
	{
		class TopBarMenu : public EditorWindow
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