#include "Platforms/Platform.h"

#include "Core/Logger.h"
#include <stdarg.h>


namespace Insight
{
	void AssetPrintError(const char* format, ...)
	{
		char buffer[2048];
		va_list args;
		va_start(args, format);
		auto w = vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);
	}
}