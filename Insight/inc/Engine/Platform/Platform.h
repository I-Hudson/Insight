#pragma once

// Link types and defines
#include "Types.h"
#include "Defines.h"

#ifdef IS_PLATFORM_WINDOWS
#include "Windows/PlatformWindows.h"
#endif

#if IS_DEBUG
// Performs hard assertion of the expression. Crashes the engine and inserts debugger break in case of expression fail.
#define ASSERT(expression) \
    if (!(expression)) \
    { \
        if (!Platform::IsDebuggerPresent()) \
        { \
            PLATFORM_DEBUG_BREAK; \
        } \
        Platform::Assert(#expression, __FILE__, __LINE__); \
    }
#else
#define ASSERT(expression) ((void)0);
#endif
#if ENABLE_ASSERTION_LOW_LAYERS
#define ASSERT_LOW_LAYER(x) ASSERT(x)
#else
#define ASSERT_LOW_LAYER(x)
#endif

// Performs soft check of the expression. Logs the expression fail to log and returns the function call.
#define CHECK(expression) \
    if (!(expression)) \
    { \
        Platform::CheckFailed(#expression, __FILE__, __LINE__); \
        return; \
    }
#define CHECK_RETURN(expression, returnValue) \
    if (!(expression)) \
    { \
        Platform::CheckFailed(#expression, __FILE__, __LINE__); \
        return returnValue; \
    }
