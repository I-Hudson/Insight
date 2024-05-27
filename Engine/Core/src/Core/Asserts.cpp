#include "Core/Asserts.h"
#include "Core/MemoryTracker.h"

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
		constexpr u64 messagebufferCallStackSize = Insight::Core::c_CallstackStringSize * Insight::Core::c_CallStackCount;
		constexpr u64 messageBufferSize = 2048;
		constexpr u64 messageBufferPaddingSize = 512;

		constexpr u64 bufferSize = messageBufferSize + messagebufferCallStackSize + messageBufferPaddingSize;
		char buffer[bufferSize];
		va_list args;
		va_start(args, format);
		u64 bufferIdx = vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		*(buffer + bufferIdx) = '\n';
		++bufferIdx;
		std::array<char[Insight::Core::c_CallstackStringSize], Insight::Core::c_CallStackCount> callStack = Core::MemoryTracker::Instance().GetCallStack();

		for (size_t i = 0; i < Insight::Core::c_CallStackCount; ++i)
		{
			const char* callStackString = callStack[i];
			const u64 callStackLen = strlen(callStackString);
			Platform::MemCopy(buffer + bufferIdx, callStackString, callStackLen);
			bufferIdx += callStackLen;
			*(buffer + bufferIdx) = '\n';
			++bufferIdx;
		}
		*(buffer + bufferIdx) = '\0';

		MessageBoxA(nullptr, buffer, "ASSERT", MB_OK);
		DebugBreak();
#elif IS_PLATFORM_UNIX
#endif
	}
}