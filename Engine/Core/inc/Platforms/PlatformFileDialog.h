#pragma once

#include "Core/Defines.h"

#include <string>

namespace Insight
{
#define PlatformDialogFileFilter(displayText, extensionText) { #displayText, #extensionText }
    enum class PlatformFileDialogOperations
    {
        LoadFile, 
        SaveFile,

        SelectFolder,
        SelectFile,
        SelectAll,
    };
    IS_CORE CONSTEXPR const char* PlatformFileDialogOperationToString(PlatformFileDialogOperations operation);
}