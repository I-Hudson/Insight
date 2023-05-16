#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"
#include "Editor/EditorWindows/Generated/GameViewWindow_reflect_generated.h"

namespace Insight
{
    namespace Editor
    {
        REFLECT_CLASS()
        class GameViewWindow : public IEditorWindow
        {
            REFLECT_GENERATED_BODY()

        public:
            GameViewWindow();
            GameViewWindow(u32 minWidth, u32 minHeight);
            GameViewWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            ~GameViewWindow();

            virtual void Initialise() override;
            virtual void OnDraw() override;
            EDITOR_WINDOW(GameViewWindow, EditorWindowCategories::Windows);
        };
    }
}