#include "Core/EnginePaths.h"

#include "Platforms/Platform.h"

#include "FileSystem/FileSystem.h"
#include <filesystem>

#ifdef IS_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace Insight
{
    bool EnginePaths::m_initialise = false;
    std::string EnginePaths::m_rootPath;
    std::string EnginePaths::m_executablePath;
    std::string EnginePaths::m_resourcePath;

    void EnginePaths::Initialise()
    {
#ifdef IS_PLATFORM_WINDOWS
        // Windows specific
        wchar_t szPath[MAX_PATH];
        GetModuleFileNameW(NULL, szPath, MAX_PATH);
        m_executablePath = std::move(std::filesystem::path{ szPath }.parent_path().string()); // to finish the folder path with (back)slash
        m_executablePath = FileSystem::FileSystem::PathToUnix(m_executablePath);
#endif
        const u32 rootFromExePath = 3;
        m_rootPath = m_executablePath;
        for (size_t i = 0; i < rootFromExePath; ++i)
        {
            m_rootPath = std::filesystem::path{ m_rootPath }.parent_path().string();
        }

        m_resourcePath = m_rootPath + "/" + "Resources";
        m_initialise = true;
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