#pragma once

#include "Editor/HotReload/HotReloadOperation.h"

#include <string>
#include <vector>

namespace Insight::Editor
{
    class EditorWindowsOperation : public HotReloadOperation
    {
    public:
        EditorWindowsOperation();
        virtual ~EditorWindowsOperation() override;

        virtual void Reset() override;
        virtual void PreUnloadOperation() override;
        virtual void PostLoadOperation() override;

    private:
        struct EditorWindowState
        {
            EditorWindowState() { }
            EditorWindowState(std::string name, bool wasOpen)
                : Name(std::move(name)), WasOpen(wasOpen)
            { }
            std::string Name;
            bool WasOpen;
        };
        std::vector<EditorWindowState> m_windows;
    };
}