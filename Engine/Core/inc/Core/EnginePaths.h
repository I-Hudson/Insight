#pragma once

#include "Core/Defines.h"

#include <string>

namespace Insight
{
    /// @brief Static class used to get the path for all engine usage.
    class IS_CORE EnginePaths
    {
    public:

        static void Initialise();

        static std::string GetInstallDirectory();
        static std::string GetExecutablePath();
        static std::string GetResourcePath();

    private:
        static bool m_initialise;
        static std::string m_installDirectory;
        static std::string m_executablePath;
        static std::string m_resourcePath;
    };
}