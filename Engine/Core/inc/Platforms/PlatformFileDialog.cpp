#include "PlatformFileDialog.h"

namespace Insight
{
    CONSTEXPR const char* PlatformFileDialogOperationToString(PlatformFileDialogOperations operation)
    {
        switch (operation)
        {
        case Insight::PlatformFileDialogOperations::Load: return "Load";
        case Insight::PlatformFileDialogOperations::Save: return "Save";
        default:
            break;
        }
        return "";
    }
}
