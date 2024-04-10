#pragma once

#include "Core/Defines.h"

namespace Insight
{
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
	if (!(condition)) {}

#define ASSERT(condition) ASSERT_MSG(condition, "")

#define FAIL_ASSERT() ASSERT(false)
#define FAIL_ASSERT_MSG(message, ...) ASSERT_MSG(false, message, __VA_ARGS__)

#endif
}