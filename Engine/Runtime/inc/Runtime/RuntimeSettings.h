#pragma once

#include "Runtime/Defines.h"
#include "Core/Singleton.h"

#include <string>

namespace Insight
{
    namespace Runtime
    {
        struct IS_RUNTIME RuntimeSettings : Core::Singleton<RuntimeSettings>
        {
            std::string InitialWorldPath;
        };
    }
}