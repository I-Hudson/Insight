#pragma once

#include "Platform/Windows/PlatformWindows.h"
#include "Core/Logger.h"

namespace Insight
{
#ifdef IS_PLATFORM_WINDOWS
		using Platform = Windows::PlatformWindows;
#endif

#define SAFE_BREAK __debugbreak()

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
#endif
}