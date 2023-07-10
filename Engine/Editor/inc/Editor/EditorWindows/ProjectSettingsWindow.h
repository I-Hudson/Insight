#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Editor/EditorWindows/Generated/ProjectSettingsWindow_reflect_generated.h"

namespace Insight
{
    namespace Editor
    {
        REFLECT_CLASS()
        class ProjectSettingsWindow : public IEditorWindow
        {
            REFLECT_GENERATED_BODY();

        public:
            constexpr static const char* c_PSW_DefaultWorldField = "PSW_DefaultWorldField";

            ProjectSettingsWindow();
            ProjectSettingsWindow(u32 minWidth, u32 minHeight);
            ProjectSettingsWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            virtual ~ProjectSettingsWindow() override;

            EDITOR_WINDOW(ProjectSettingsWindow, EditorWindowCategories::Windows);
            virtual void OnDraw() override;
        };
    }
}