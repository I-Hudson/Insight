#pragma once

#ifdef IS_PLATFORM_WINDOWS

#include "Core/TypeAlias.h"
#include <string>
#include <vector>

namespace Insight
{
	namespace Windows
	{
		class PlatformWindows
		{
		public:

			static void MemCopy(void* dst, void const* src, u64 size);
			static void MemSet(void* dst, int value, u64 size);
			static void MemClear(void* dst, u64 size);
			static bool MemCompare(const void* buf1, const void* buf2, u64 size);

			static std::vector<std::string> GetCallStack(int frameCount = 32);
		};

	}
}
#endif // IS_PLATFORM_WINDOWS