#pragma once

#include "Types.h"

#if IS_PLATFORM_WINDOWS
#include "Win32/Win32CriticalSection.h"
#else

#error CriticalSection no platform support.

#endif