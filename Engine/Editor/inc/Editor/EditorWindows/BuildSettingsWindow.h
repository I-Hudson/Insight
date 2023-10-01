#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"
#include "Editor/EditorWindows/Generated/BuildSettingsWindow_reflect_generated.h"

namespace Insight
{
    namespace Editor
    {
		class BuildSystem;

		REFLECT_CLASS()
        class BuildSettingsWindow : public IEditorWindow
		{
			REFLECT_GENERATED_BODY()

		public:
			BuildSettingsWindow();
			BuildSettingsWindow(u32 minWidth, u32 minHeight);
			BuildSettingsWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			~BuildSettingsWindow();

			// - Begin IEditorWindow -
			virtual void Initialise() override;
			virtual void Shutdown() override;
			virtual void OnDraw() override;
			// - End IEditorWindow -

			EDITOR_WINDOW(BuildSettingsWindow, EditorWindowCategories::File);

		private:
			BuildSystem* m_buildSystem = nullptr;
        };
    }
}