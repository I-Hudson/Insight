#pragma once

#include "Core/Defines.h"

#include <string>

namespace Insight
{
    namespace FileSystem
    {
        class IS_CORE FileSystem
        {
        public:

            static std::string GetAbsolutePath(const std::string& path);
            static std::string PathToUnix(std::string& path);
        };
    }
}