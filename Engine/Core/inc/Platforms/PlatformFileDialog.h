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
        FileDialogFilter() { }
        FileDialogFilter(const wchar_t* name, const wchar_t* extension)
            : Name(name), Extension(extension)
        { }
        const wchar_t* Name = L"";
        const wchar_t* Extension = L"";
    };

    IS_CORE CONSTEXPR const char* PlatformFileDialogOperationToString(PlatformFileDialogOperations operation);
}