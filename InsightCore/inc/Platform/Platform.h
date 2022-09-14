#pragma once

#ifdef IS_PLATFORM_WINDOWS
#include "Platform/Windows/PlatformWindows.h"
#define SAFE_BREAK __debugbreak()
#elif IS_PLATFORM_LINUX
#define SAFE_BREAK(void)
#endif
#include "Core/Logger.h"

namespace Insight
{
#ifdef IS_PLATFORM_WINDOWS
	using Platform = Windows::PlatformWindows;
#elif IS_PLATFORM_LINUX
	using Platform = Linux::PlatformLinux;
#endif

#define IS_ASSERT_ENABLED
#ifdef IS_ASSERT_ENABLED
#define ASSERT(condition)																									\
	if (!(condition))																										\
	{																														\
		IS_CORE_ERROR("ASSERT: \n\tFILE: '{0}', \n\tLINE: '{1}', \n\tCondition: '{2}'", __FILE__, __LINE__, #condition);	\
		SAFE_BREAK;																											\
	}

#define ASSERT_MSG(condition, message)																													\
	if (!(condition))																																	\
	{																																					\
		IS_CORE_ERROR("ASSERT: \n\tFILE: '{0}', \n\tLINE: '{1}', \n\tCondition: '{2}', \n\tMessage: '{4}'", __FILE__, __LINE__, #condition, message);	\
		SAFE_BREAK;																																		\
	}

#define FAIL_ASSERT(condition)																									\
		ASSERT(false)

#define FAIL_ASSERT_MSG(message)																									\
		ASSERT_MSG(false, message)
#endif
}