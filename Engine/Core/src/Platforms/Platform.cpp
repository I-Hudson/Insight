#include "Platforms/Platform.h"

#include <stdarg.h>
#include <iostream>
#include <Windows.h>

namespace Insight
{
	void AssetPrintError(const char* format, ...)
	{
		char buffer[2048];
		va_list args;
		va_start(args, format);
		auto w = vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

#ifdef IS_PLATFORM_WINDOWS
		MessageBoxA(nullptr, buffer, "ASSERT", MB_OK);
		DebugBreak();
#elif IS_PLATFORM_UNIX
#endif
	}
}