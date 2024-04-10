#include "Core/EnginePaths.h"

#include "Core/Asserts.h"

#include "FileSystem/FileSystem.h"
#include <filesystem>

#ifdef IS_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace Insight
{
    bool EnginePaths::m_initialise = false;
    std::string EnginePaths::m_installDirectory;
    std::string EnginePaths::m_executablePath;
    std::string EnginePaths::m_resourcePath;

    void EnginePaths::Initialise()
    {
#ifdef IS_PLATFORM_WINDOWS
        // Windows specific
        wchar_t szPath[MAX_PATH];
        GetModuleFileNameW(NULL, szPath, MAX_PATH);
        m_executablePath = std::move(std::filesystem::path{ szPath }.parent_path().string()); // to finish the folder path with (back)slash
        FileSystem::PathToUnix(m_executablePath);
#endif

        const u32 installDirectoryFromExePath = 3;
        m_installDirectory = m_executablePath;
        for (size_t i = 0; i < installDirectoryFromExePath; ++i)
        {
            m_installDirectory = std::filesystem::path{ m_installDirectory }.parent_path().string();
        }

        m_resourcePath = m_executablePath + "/" + "Resources";
        m_initialise = true;
    }

    std::string EnginePaths::GetInstallDirectory()
    {
        ASSERT(m_initialise);
        return m_installDirectory;
    }

    std::string EnginePaths::GetExecutablePath()
    {
        ASSERT(m_initialise);
        return m_executablePath;
    }

    std::string EnginePaths::GetResourcePath()
    {
        ASSERT(m_initialise);
        return m_resourcePath;
    }
}