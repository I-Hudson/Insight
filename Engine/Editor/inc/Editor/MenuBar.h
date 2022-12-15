#pragma once

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
            EditorWindowManager* m_editorWindowManager;
        };
    }
}