#pragma once

#include "Core/Defines.h"

#include <string>

namespace Insight
{
    enum class PlatformFileDialogOperations
    {
        Load, 
        Save
    };
    IS_CORE CONSTEXPR const char* PlatformFileDialogOperationToString(PlatformFileDialogOperations operation);
}