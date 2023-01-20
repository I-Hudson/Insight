#include "PlatformFileDialog.h"

namespace Insight
{
    CONSTEXPR const char* PlatformFileDialogOperationToString(PlatformFileDialogOperations operation)
    {
        switch (operation)
        {
        case Insight::PlatformFileDialogOperations::LoadFile:       return "LoadFile";
        case Insight::PlatformFileDialogOperations::SaveFile:       return "SaveFile";
        case Insight::PlatformFileDialogOperations::SelectFolder:   return "SelectFolder";
        case Insight::PlatformFileDialogOperations::SelectFile:     return "SelectFile";
        case Insight::PlatformFileDialogOperations::SelectAll:      return "SelectAll";
        default:
            break;
        }
        return "";
    }
}
