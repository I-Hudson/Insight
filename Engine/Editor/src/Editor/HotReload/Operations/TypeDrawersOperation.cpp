#include "Editor/HotReload/Operations/TypeDrawersOperation.h"
#include "Editor/TypeDrawers/TypeDrawerRegister.h"

namespace Insight::Editor
{
    TypeDrawersOperation::TypeDrawersOperation()
    {
    }

    TypeDrawersOperation::~TypeDrawersOperation()
    {
    }

    void TypeDrawersOperation::Reset()
    {
    }

    void TypeDrawersOperation::PreUnloadOperation()
    {
        /*const HotReloadLibrary& library = HotReloadSystem::Instance().GetLibrary();
        HotReloadMetaData metaData = library.GetMetaData();

        EditorWindowManager& editorWindowManager = EditorWindowManager::Instance();
        for (const std::string& windowName : metaData.EditorWindowNames)
        {
            bool isOpen = editorWindowManager.IsWindowVisable(windowName);
            m_windows.push_back(EditorWindowState(windowName, isOpen));
            editorWindowManager.RemoveWindowNow(windowName);
        }*/
    }

    void TypeDrawersOperation::PostLoadOperation()
    {
        /*EditorWindowManager& editorWindowManager = EditorWindowManager::Instance();
        for (const EditorWindowState& windowState : m_windows)
        {
            if (windowState.WasOpen)
            {
                editorWindowManager.AddWindow(windowState.Name);
            }
        }*/
    }
}