#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

namespace Insight
{
    namespace Editor
    {
        class BuildSettingsWindow : public IEditorWindow
		{
		public:
			BuildSettingsWindow();
			BuildSettingsWindow(u32 minWidth, u32 minHeight);
			BuildSettingsWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			~BuildSettingsWindow();

			virtual void OnDraw() override;
			EDITOR_WINDOW(BuildSettingsWindow, EditorWindowCategories::File);
        };
    }
}