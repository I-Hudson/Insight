#pragma once

#include "Platform/Windows/PlatformWindows.h"

namespace Insight
{
#ifdef IS_PLATFORM_WINDOWS
		using Platform = Windows::PlatformWindows;
#endif
}