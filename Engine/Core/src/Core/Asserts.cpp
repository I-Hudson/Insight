#include "Core/Asserts.h"

#ifdef IS_PLATFORM_WINDOWS
#include <stdarg.h>
#include <iostream>
#include <Windows.h>
#elif IS_PLATFORM_UNIX
#endif

namespace Insight
{
	void AssertPrintError(const char* format, ...)
	{
#ifdef IS_PLATFORM_WINDOWS
		char buffer[2048];
		va_list args;
		va_start(args, format);
		auto w = vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		MessageBoxA(nullptr, buffer, "ASSERT", MB_OK);
		DebugBreak();
#elif IS_PLATFORM_UNIX
#endif
	}
}