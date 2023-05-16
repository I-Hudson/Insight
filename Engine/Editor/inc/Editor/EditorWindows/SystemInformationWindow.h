#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"
#include "Editor/EditorWindows/Generated/SystemInformationWindow_reflect_generated.h"

namespace Insight
{
    namespace Editor
    {
        REFLECT_CLASS()
        class SystemInformationWindow : public IEditorWindow
        {
            REFLECT_GENERATED_BODY()

        public:
            SystemInformationWindow();
            SystemInformationWindow(u32 minWidth, u32 minHeight);
            SystemInformationWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            virtual ~SystemInformationWindow() override;

            EDITOR_WINDOW(SystemInformationWindow, EditorWindowCategories::Windows);

            // IEditorWindow
            virtual void OnDraw() override;
        };
    }
}