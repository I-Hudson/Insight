#pragma once

#include "Core/Defines.h"

#include <string>

namespace Insight
{
    enum class PlatformFileDialogOperations
    {
        LoadFile, 
        SaveFile,

        SelectFolder,
        SelectFile,
        SelectAll,
    };

    struct FileDialogFilter
    {
        const wchar_t* Name;
        const wchar_t* Extension;
    };

    IS_CORE CONSTEXPR const char* PlatformFileDialogOperationToString(PlatformFileDialogOperations operation);
}