// Copyright (c) 2012-2021 Wojciech Figat. All rights reserved.

#pragma once

/// <summary>
/// The platform the game is running.
/// </summary>
enum class PlatformType
{
    /// <summary>
    /// Running on Windows.
    /// </summary>
    Windows = 1,

    /// <summary>
    /// Running on Xbox One.
    /// </summary>
    XboxOne = 2,

    /// <summary>
    /// Running Windows Store App (Universal Windows Platform).
    /// </summary>
    UWP = 3,

    /// <summary>
    /// Running on Linux system.
    /// </summary>
    Linux = 4,

    /// <summary>
    /// Running on PlayStation 4.
    /// </summary>
    PS4 = 5,

    /// <summary>
    /// Running on Xbox Series X.
    /// </summary>
    XboxScarlett = 6,

    /// <summary>
    /// Running on Android.
    /// </summary>
    Android = 7,
};

/// <summary>
/// The platform architecture types.
/// </summary>
enum class ArchitectureType
{
    /// <summary>
    /// Anything or not important.
    /// </summary>
    AnyCPU = 0,

    /// <summary>
    /// The x86 32-bit.
    /// </summary>
    x86 = 1,

    /// <summary>
    /// The x86 64-bit.
    /// </summary>
    x64 = 2,

    /// <summary>
    /// The ARM 32-bit.
    /// </summary>
    ARM = 3,

    /// <summary>
    /// The ARM 64-bit.
    /// </summary>
    ARM64 = 4,
};

// Add missing platforms definitions
#if !defined(IS_PLATFORM_WINDOWS)
#define IS_PLATFORM_WINDOWS 0
#endif
#if !defined(IS_PLATFORM_UWP)
#define IS_PLATFORM_UWP 0
#endif
#if !defined(IS_PLATFORM_WIN32)
#define IS_PLATFORM_WIN32 0
#endif
#if !defined(IS_PLATFORM_XBOX_ONE)
#define IS_PLATFORM_XBOX_ONE 0
#endif
#if !defined(IS_PLATFORM_UNIX)
#define IS_PLATFORM_UNIX 0
#endif
#if !defined(IS_PLATFORM_LINUX)
#define IS_PLATFORM_LINUX 0
#endif
#if !defined(IS_PLATFORM_PS4)
#define IS_PLATFORM_PS4 0
#endif
#if !defined(IS_PLATFORM_XBOX_SCARLETT)
#define IS_PLATFORM_XBOX_SCARLETT 0
#endif
#if !defined(IS_PLATFORM_ANDROID)
#define IS_PLATFORM_ANDROID 0
#endif
#if !defined(IS_PLATFORM_MAC)
#define IS_PLATFORM_MAC 0
#endif
#if !defined(IS_PLATFORM_OSX)
#define IS_PLATFORM_OSX 0
#endif
#if !defined(IS_PLATFORM_IOS)
#define IS_PLATFORM_IOS 0
#endif

#if IS_PLATFORM_WINDOWS
#include "Windows/WindowsDefines.h"
#elif IS_PLATFORM_UWP
#include "UWP/UWPDefines.h"
#elif IS_PLATFORM_LINUX
#include "Linux/LinuxDefines.h"
#elif IS_PLATFORM_PS4
#include "Platforms/PS4/Engine/Platform/PS4Defines.h"
#elif IS_PLATFORM_XBOX_SCARLETT
#include "Platforms/XboxScarlett/Engine/Platform/XboxScarlettDefines.h"
#elif IS_PLATFORM_ANDROID
#include "Android/AndroidDefines.h"
#else
#error Missing Defines implementation!
#endif

// Create default definitions if any missing
#ifndef IS_PLATFORM_64BITS
#define IS_PLATFORM_64BITS 0
#endif
#ifndef IS_PLATFORM_DESKTOP
#define IS_PLATFORM_DESKTOP 0
#endif
#ifndef IS_PLATFORM_ARCH_X64
#define IS_PLATFORM_ARCH_X64 0
#endif
#ifndef IS_PLATFORM_ARCH_X86
#define IS_PLATFORM_ARCH_X86 0
#endif
#ifndef IS_PLATFORM_ARCH_ARM
#define IS_PLATFORM_ARCH_ARM 0
#endif
#ifndef IS_PLATFORM_ARCH_ARM64
#define IS_PLATFORM_ARCH_ARM64 0
#endif
#ifndef IS_PLATFORM_WCHAR_IS_CHAR16
#define IS_PLATFORM_WCHAR_IS_CHAR16 0
#endif
#ifndef IS_PLATFORM_DEBUG_BREAK
#define IS_PLATFORM_DEBUG_BREAK
#endif
#ifndef IS_PLATFORM_LINE_TERMINATOR
#define IS_PLATFORM_LINE_TERMINATOR "\n"
#endif
#define IS_PLATFORM_32BITS (!IS_PLATFORM_64BITS)

// Platform family defines
#define IS_PLATFORM_WINDOWS_FAMILY (IS_PLATFORM_WINDOWS || IS_PLATFORM_UWP)
#define IS_PLATFORM_MICROSOFT_FAMILY (IS_PLATFORM_WINDOWS_FAMILY || IS_PLATFORM_XBOX_ONE)
#define IS_PLATFORM_UNIX_FAMILY (IS_PLATFORM_LINUX || IS_PLATFORM_ANDROID || IS_PLATFORM_PS4)
#define IS_PLATFORM_APPLE_FAMILY (IS_PLATFORM_IOS || IS_PLATFORM_OSX)

// SIMD defines
#if defined(__i386__) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64) || (defined (__EMSCRIPTEN__) && defined(__SSE2__))
#define IS_PLATFORM_SIMD_SSE2 1
#endif
#if defined(_M_ARM) || defined(__ARM_NEON__) || defined(__ARM_NEON)
#define IS_PLATFORM_SIMD_NEON 1
#endif
#if defined(_M_PPC) || defined(__CELLOS_LV2__)
#define IS_PLATFORM_SIMD_VMX 1
#endif

// Unicode text macro
#if !defined(TEXT)
#if IS_PLATFORM_TEXT_IS_CHAR16
#define _TEXT(x) u ## x
#else
#define _TEXT(x) L ## x
#endif
#define TEXT(x) _TEXT(x)
#endif

// Static array size
#define ARRAY_COUNT(array) (sizeof(array) / sizeof(array[0]))
