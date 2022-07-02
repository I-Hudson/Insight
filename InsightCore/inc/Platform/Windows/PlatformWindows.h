#pragma once

#ifdef IS_PLATFORM_WINDOWS

#include "Core/TypeAlias.h"
#include "Core/Defines.h"
#include "Core/GUID.h"

#include <string>
#include <vector>

namespace Insight
{
	namespace Windows
	{
		class IS_CORE PlatformWindows
		{
		public:

			static void MemCopy(void* dst, void const* src, u64 size);
			static void MemSet(void* dst, int value, u64 size);
			static void MemClear(void* dst, u64 size);
			static bool MemCompare(const void* buf1, const void* buf2, u64 size);
			static bool StrCompare(const char* str1, const char* str2);
			static bool WStrCompare(const wchar_t* wstr1, const wchar_t* wstr2);

			static std::vector<std::string> GetCallStack(int frameCount = 32);

			static Core::GUID CreateGUID();
			static void AssignGUID(Core::GUID& guid);
		};

	}
}
#endif // IS_PLATFORM_WINDOWS