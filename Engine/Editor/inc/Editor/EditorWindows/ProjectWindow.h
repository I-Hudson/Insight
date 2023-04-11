#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

namespace Insight
{
    namespace Editor
    {
        REFLECT_CLASS()
        class ProjectWindow : public IEditorWindow
        {
        public:
            ProjectWindow();
            ProjectWindow(u32 minWidth, u32 minHeight);
            ProjectWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            ~ProjectWindow();

            virtual void OnDraw() override;
            EDITOR_WINDOW(ProjectWindow, EditorWindowCategories::File);
        };
    }
}