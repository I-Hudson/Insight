#include "Editor/HotReload/Operations/EditorWindowsOperation.h"
#include "Editor/HotReload/HotReloadSystem.h"

#include "Editor/EditorWindows/EditorWindowManager.h"

namespace Insight::Editor
{
    EditorWindowsOperation::EditorWindowsOperation()
    {
    }

    EditorWindowsOperation::~EditorWindowsOperation()
    {
    }

    void EditorWindowsOperation::Reset()
    {
        m_windows.clear();
    }

    void EditorWindowsOperation::PreUnloadOperation()
    {
        const HotReloadLibrary& library = HotReloadSystem::Instance().GetLibrary();
        HotReloadMetaData metaData = library.GetMetaData();

        EditorWindowManager& editorWindowManager = EditorWindowManager::Instance();
        for (const std::string& windowName : metaData.EditorWindowNames)
        {
            bool isOpen = editorWindowManager.IsWindowVisable(windowName);
            m_windows.push_back(EditorWindowState(windowName, isOpen));
            editorWindowManager.RemoveWindowNow(windowName);
        }
    }

    void EditorWindowsOperation::PostLoadOperation()
    {
        EditorWindowManager& editorWindowManager = EditorWindowManager::Instance();
        for (const EditorWindowState& windowState : m_windows)
        {
            if (windowState.WasOpen)
            {
                editorWindowManager.AddWindow(windowState.Name);
            }
        }
    }
}