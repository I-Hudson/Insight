#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

namespace Insight
{
    namespace Editor
    {
        class SystemInformationWindow : public IEditorWindow
        {
        public:
            SystemInformationWindow();
            SystemInformationWindow(u32 minWidth, u32 minHeight);
            SystemInformationWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            virtual ~SystemInformationWindow() override;

            EDITOR_WINDOW(SystemInformationWindow);

            // IEditorWindow
            virtual void OnDraw() override;
        };
    }
}