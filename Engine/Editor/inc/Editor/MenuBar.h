#pragma once

#include "Editor/FileDialog.h"
#include "Editor/EditorWindows/IEditorWindow.h"

namespace Insight
{
    namespace Editor
    {
        class EditorWindowManager;

        class MenuBar
        {
        public:
            MenuBar();
            ~MenuBar();

            void Initialise(EditorWindowManager* editorWindowManager);

            void Draw();

        private:
            void DrawAllRegisteredWindow(EditorWindowCategories category) const;

        private:
            EditorWindowManager* m_editorWindowManager;
            FileDialog m_fileDialog;
        };
    }
}