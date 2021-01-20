// Copyright (c) 2012-2021 Wojciech Figat. All rights reserved.

#pragma once

#if IS_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Platform/Win32/Win32Platform.h"
#include "glm/glm.hpp"

/// <summary>
/// The Windows platform implementation and application management utilities.
/// </summary>
class IS_API WindowsPlatform : public Win32Platform
{
public:

    /// <summary>
    /// Win32 application windows class name.
    /// </summary>
    static const WChar* ApplicationWindowClass;

    /// <summary>
    /// Handle to Win32 application instance.
    /// </summary>
    static void* Instance;

public:

    /// <summary>
    /// Creates the system-wide mutex.
    /// </summary>
    /// <param name="name">The name of the mutex.</param>
    /// <returns>True if mutex already exists, otherwise false.</returns>
    static bool CreateMutex(const WChar* name);

    /// <summary>
    /// Releases the mutex.
    /// </summary>
    static void ReleaseMutex();

    /// <summary>
    /// Pre initialize platform.
    /// </summary>
    /// <param name="hInstance">The Win32 application instance.</param>
    static void PreInit(void* hInstance);

    /// <summary>
    /// Returns true if current OS version is Windows 10.
    /// </summary>
    /// <returns>True if running on Windows 10 or later, otherwise false.</returns>
    static bool IsWindows10();

    static bool ReadRegValue(void* root, const std::string& key, const std::string& name, std::string* result);

public:

    // [Win32Platform]
    static bool Init();
    static void LogInfo();
    static void Tick();
    static void BeforeExit();
    static void Exit();
#if IS_DEBUG
    static void Log(const std::string_view &msg);
    static bool IsDebuggerPresent();
#endif
    static void SetHighDpiAwarenessEnabled(bool enable);
    static I32 GetDpi();
    static std::string GetUserLocaleName();
    static std::string GetComputerName();
    static std::string GetUserName();
    static bool GetHasFocus();
    static bool CanOpenUrl(const std::string_view& url);
    static void OpenUrl(const std::string_view& url);
    static glm::vec4 GetMonitorBounds(const glm::vec2& screenPos);
    static glm::vec2 GetDesktopSize();
    static glm::vec4 GetVirtualDesktopBounds();
    static void GetEnvironmentVariables(std::unordered_map<std::string, std::string>& result);
    static bool GetEnvironmentVariable(const std::string& name, std::string& value);
    static bool SetEnvironmentVariable(const std::string& name, const std::string& value);
    static I32 StartProcess(const std::string_view& filename, const std::string_view& args, const std::string_view& workingDir, bool hiddenWindow = false, bool waitForEnd = false);
    static I32 RunProcess(const std::string_view& cmdLine, const std::string_view& workingDir, bool hiddenWindow = true);
    static I32 RunProcess(const std::string_view& cmdLine, const std::string_view& workingDir, const std::unordered_map<std::string, std::string>& environment, bool hiddenWindow = true);
    static Window* CreatePlatformWindow(const CreateWindowSettings& settings);
};

#endif
