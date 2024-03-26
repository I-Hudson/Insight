#pragma once

#ifdef IS_PLATFORM_WINDOWS
#include "Platforms/Windows/PlatformWindows.h"
#include "Platforms/Windows/PlatformFileDialogWindows.h"
#include "Platforms/Windows/PlatformProgressWindows.h"
#elif IS_PLATFORM_LINUX
#define SAFE_BREAK(void)
#endif

#include "Core/Defines.h"

#if defined(IS_PLATFORM_WINDOWS) || defined(IS_PLATFORM_OSX) ||defined(IS_PLATFORM_LINUX)
#define IS_PLATFORM_DESKTOP
#endif

namespace Insight
{
#ifdef IS_PLATFORM_WINDOWS
	using Platform = Windows::PlatformWindows;
	using PlatformFileDialog = Platforms::Windows::PlatformFileDialogWindows;
	using PlatformProgress = Platforms::Windows::PlatformProgressWindows;
#elif IS_PLATFORM_LINUX
	using Platform = Linux::PlatformLinux;
	using PlatformFileDialog = Windows::PlatformFileDialogLinux;
#endif

#define IS_ASSERT_ENABLED
#ifdef IS_ASSERT_ENABLED

	void IS_CORE AssetPrintError(const char* format, ...);

#define ASSERT_MSG(condition, message, ...)																												\
	if (!(condition))																																	\
	{																																					\
		AssetPrintError("ASSERT:\nFILE: '%s', \nLINE: '%i', \nCondition: '%s', \nMessage: '%s'", __FILE__, __LINE__, #condition, message, __VA_ARGS__);																													\
	}

#define ASSERT(condition) ASSERT_MSG(condition, "")

#define FAIL_ASSERT() ASSERT(false)
#define FAIL_ASSERT_MSG(message, ...) ASSERT_MSG(false, message, __VA_ARGS__)
#else
	#define ASSERT_MSG(condition, message, ...)
	if (!(condition)) { }

#define ASSERT(condition) ASSERT_MSG(condition, "")

#define FAIL_ASSERT() ASSERT(false)
#define FAIL_ASSERT_MSG(message, ...) ASSERT_MSG(false, message, __VA_ARGS__)

#endif
}